import type { Ref } from 'vue'

type PointUpdate = {
  event: string;
  name: string;
  value: string;
  timestamp: number;
  driver?: string;
  device?: string;
}

class WSClient {
  private url: string;
  private ws: WebSocket | null = null;
  private reconnectMs = 3000;
  private shouldReconnect = true;
  private handlers: Array<(msg: PointUpdate)=>void> = [];
  private openCbs: Array<()=>void> = [];

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
    }
    this.ws.onmessage = (ev)=>{
      try{
        const text = ev.data as string;
        const obj = JSON.parse(text) as PointUpdate;
        if (obj && obj.event === 'POINT_UPDATE'){
          this.handlers.forEach(h=>{ try{ h(obj); }catch(e){} });
        }
      }catch(e){
        // ignore non-json or other messages
      }
    }
    this.ws.onclose = ()=>{
      console.warn('[WS] closed')
      this.ws = null;
      if (this.shouldReconnect){
        setTimeout(()=>this.connect(), this.reconnectMs);
      }
    }
    this.ws.onerror = ()=>{
      console.error('[WS] error')
      if (this.ws) this.ws.close();
    }
  }

  stop(){ this.shouldReconnect = false; if (this.ws) this.ws.close(); }

  subscribePoint(id: string){
    if (this.ws && this.ws.readyState === WebSocket.OPEN){
      this.ws.send(`SUBSCRIBE ${id}`);
    } else {
      // send after open
      const cb = () => { if (this.ws && this.ws.readyState === WebSocket.OPEN) this.ws!.send(`SUBSCRIBE ${id}`); this.offOpen(cb); };
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

  addHandler(h:(m:PointUpdate)=>void): void { this.handlers.push(h); }
  removeHandler(h:(m:PointUpdate)=>void): void { this.handlers = this.handlers.filter(x=>x!==h); }
}

export const wsClient = new WSClient();
