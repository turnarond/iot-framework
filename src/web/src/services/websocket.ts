import type { Ref } from 'vue'
import { pointService } from './pointService'

export type PointUpdate = {
  event: string;
  name: string;
  value: string;
  timestamp: number;
  driver?: string;
  device?: string;
}

export type SystemStatusUpdate = {
  event: string;
  cpu: number;
  memory: number;
  disk: number;
  network: number;
  timestamp: number;
}

export type DeviceStatusUpdate = {
  event: string;
  deviceId: string;
  status: 'online' | 'offline' | 'error';
  responseTime: number;
  timestamp: number;
}

export type AlarmUpdate = {
  event: string;
  alarmId: string;
  type: string;
  level: 'info' | 'warning' | 'error';
  message: string;
  timestamp: number;
  deviceId?: string;
  pointName?: string;
}

export type WSEvent = PointUpdate | SystemStatusUpdate | DeviceStatusUpdate | AlarmUpdate;

export class WSClient {
  private url: string;
  private ws: WebSocket | null = null;
  private reconnectMs = 3000;
  private shouldReconnect = true;
  private handlers: Map<string, Array<(msg: WSEvent)=>void>> = new Map();
  private openCbs: Array<()=>void> = [];
  private heartbeatInterval: number | null = null;

  constructor(url = 'ws://localhost:9000'){
    this.url = url;
    this.connect();
  }

  connect(){    
    if (this.ws) return;
    this.ws = new WebSocket(this.url);
    this.ws.onopen = ()=>{
      console.info('[WS] open')
      // call pending open callbacks
      try{
        this.openCbs.forEach(cb=>{ try{ cb() }catch(e){} })
      }catch(e){}
      this.openCbs = []
      // 启动心跳
      this.startHeartbeat();
    }
    this.ws.onmessage = (ev)=>{
      try{
        const text = ev.data as string;
        const obj = JSON.parse(text) as WSEvent;
        if (obj && obj.event){
          // 处理不同类型的事件
          this.handleEvent(obj);
        }
      }catch(e){
        // ignore non-json or other messages
        console.warn('[WS] invalid message:', e);
      }
    }
    this.ws.onclose = ()=>{
      console.warn('[WS] closed')
      this.ws = null;
      // 停止心跳
      this.stopHeartbeat();
      if (this.shouldReconnect){
        setTimeout(()=>this.connect(), this.reconnectMs);
      }
    }
    this.ws.onerror = (error)=>{
      console.error('[WS] error:', error)
      if (this.ws) this.ws.close();
    }
  }

  stop(){ 
    this.shouldReconnect = false;
    this.stopHeartbeat();
    if (this.ws) this.ws.close();
  }

  // 启动心跳
  private startHeartbeat(){
    this.heartbeatInterval = window.setInterval(()=>{
      if (this.ws && this.ws.readyState === WebSocket.OPEN){
        this.ws.send('PING');
      }
    }, 30000); // 30秒心跳
  }

  // 停止心跳
  private stopHeartbeat(){
    if (this.heartbeatInterval){
      clearInterval(this.heartbeatInterval);
      this.heartbeatInterval = null;
    }
  }

  // 处理事件
  private handleEvent(event: WSEvent){
    // 处理所有事件的通用处理器
    const allHandlers = this.handlers.get('*') || [];
    allHandlers.forEach(h=>{ try{ h(event); }catch(e){ console.error('[WS] handler error:', e); } });
    
    // 处理特定事件类型的处理器
    const typeHandlers = this.handlers.get(event.event) || [];
    typeHandlers.forEach(h=>{ try{ h(event); }catch(e){ console.error('[WS] handler error:', e); } });
    
    // 处理点位更新事件，传递给pointService
    if (event.event === 'POINT_UPDATE') {
      pointService.handlePointUpdate(event as PointUpdate);
    }
  }

  subscribePoint(id: string){
    if (this.ws && this.ws.readyState === WebSocket.OPEN){
      this.ws.send(`SUBSCRIBE ${id}`);
    } else {
      // send after open
      const cb = () => { 
        if (this.ws && this.ws.readyState === WebSocket.OPEN) 
          this.ws!.send(`SUBSCRIBE ${id}`); 
        this.offOpen(cb); 
      };
      this.onOpen(cb);
    }
  }

  unsubscribePoint(id: string){
    if (this.ws && this.ws.readyState === WebSocket.OPEN){
      this.ws.send(`UNSUBSCRIBE ${id}`);
    } else {
      // send after open
      const cb = () => { 
        if (this.ws && this.ws.readyState === WebSocket.OPEN) 
          this.ws!.send(`UNSUBSCRIBE ${id}`); 
        this.offOpen(cb); 
      };
      this.onOpen(cb);
    }
  }

  // 订阅系统状态
  subscribeSystemStatus(){
    if (this.ws && this.ws.readyState === WebSocket.OPEN){
      this.ws.send('SUBSCRIBE SYSTEM_STATUS');
    } else {
      const cb = () => { 
        if (this.ws && this.ws.readyState === WebSocket.OPEN) 
          this.ws!.send('SUBSCRIBE SYSTEM_STATUS'); 
        this.offOpen(cb); 
      };
      this.onOpen(cb);
    }
  }

  // 订阅设备状态
  subscribeDeviceStatus(deviceId?: string){
    if (this.ws && this.ws.readyState === WebSocket.OPEN){
      this.ws.send(`SUBSCRIBE DEVICE_STATUS${deviceId ? ` ${deviceId}` : ''}`);
    } else {
      const cb = () => { 
        if (this.ws && this.ws.readyState === WebSocket.OPEN) 
          this.ws!.send(`SUBSCRIBE DEVICE_STATUS${deviceId ? ` ${deviceId}` : ''}`); 
        this.offOpen(cb); 
      };
      this.onOpen(cb);
    }
  }

  // 订阅报警
  subscribeAlarms(level?: 'info' | 'warning' | 'error'){
    if (this.ws && this.ws.readyState === WebSocket.OPEN){
      this.ws.send(`SUBSCRIBE ALARMS${level ? ` ${level}` : ''}`);
    } else {
      const cb = () => { 
        if (this.ws && this.ws.readyState === WebSocket.OPEN) 
          this.ws!.send(`SUBSCRIBE ALARMS${level ? ` ${level}` : ''}`); 
        this.offOpen(cb); 
      };
      this.onOpen(cb);
    }
  }

  private onOpen(cb: ()=>void){
    if (!this.ws) { this.openCbs.push(cb); return; }
    if (this.ws.readyState === WebSocket.OPEN) cb(); else this.openCbs.push(cb);
  }
  private offOpen(cb: ()=>void){
    this.openCbs = this.openCbs.filter(x=>x!==cb);
  }

  // 添加事件处理器
  addHandler(eventType: string, h:(m: WSEvent)=>void): void {
    if (!this.handlers.has(eventType)){
      this.handlers.set(eventType, []);
    }
    this.handlers.get(eventType)?.push(h);
  }

  // 移除事件处理器
  removeHandler(eventType: string, h:(m: WSEvent)=>void): void {
    const handlers = this.handlers.get(eventType);
    if (handlers){
      this.handlers.set(eventType, handlers.filter(x=>x!==h));
    }
  }

  // 发送自定义消息
  sendMessage(message: string): void {
    if (this.ws && this.ws.readyState === WebSocket.OPEN){
      this.ws.send(message);
    } else {
      const cb = () => { 
        if (this.ws && this.ws.readyState === WebSocket.OPEN) 
          this.ws!.send(message); 
        this.offOpen(cb); 
      };
      this.onOpen(cb);
    }
  }

  // 获取连接状态
  get readyState(): number {
    return this.ws?.readyState || WebSocket.CLOSED;
  }

  // 重连
  reconnect(): void {
    this.stop();
    this.shouldReconnect = true;
    this.connect();
  }
}

export const wsClient = new WSClient();
