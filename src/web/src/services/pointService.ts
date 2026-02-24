import axios from 'axios';
import { wsClient, type PointUpdate } from './websocket';

/**
 * 点位服务
 * 处理点位相关的API请求和WebSocket订阅管理
 */
export class PointService {
  private static instance: PointService;
  private pointHandlers: Map<string, Array<(point: PointUpdate) => void>> = new Map();
  private allPoints: any[] = [];
  private loading: boolean = false;
  private error: string = '';

  private constructor() {}

  /**
   * 获取单例实例
   */
  public static getInstance(): PointService {
    if (!PointService.instance) {
      PointService.instance = new PointService();
    }
    return PointService.instance;
  }

  /**
   * 获取所有点位列表
   */
  public async fetchAllPoints(prefix: string = ''): Promise<any[]> {
    this.loading = true;
    this.error = '';

    try {
      const response = await axios.get(`http://localhost:8081/api/v1/points?prefix=${prefix}`);
      if (response.data && response.data.points) {
        this.allPoints = Object.keys(response.data.points).map(name => ({
          name,
          ...response.data.points[name]
        }));
      }
      return this.allPoints;
    } catch (error) {
      this.error = `获取点位列表失败: ${error instanceof Error ? error.message : '未知错误'}`;
      console.error('Error fetching points:', error);
      return [];
    } finally {
      this.loading = false;
    }
  }

  /**
   * 订阅点位
   */
  public subscribePoint(pointName: string): void {
    wsClient.subscribePoint(pointName);
    console.log(`订阅点位: ${pointName}`);
  }

  /**
   * 取消订阅点位
   */
  public unsubscribePoint(pointName: string): void {
    wsClient.unsubscribePoint(pointName);
    console.log(`取消订阅点位: ${pointName}`);
  }

  /**
   * 批量订阅点位
   */
  public subscribePoints(pointNames: string[]): void {
    pointNames.forEach(name => this.subscribePoint(name));
  }

  /**
   * 发送控制命令
   */
  public async sendControlCommand(pointId: string, value: string): Promise<boolean> {
    try {
      const response = await axios.post('http://localhost:8081/api/v1/control', {
        pointId,
        value
      });
      return response.data && response.data.success;
    } catch (error) {
      console.error('控制命令发送失败:', error);
      return false;
    }
  }

  /**
   * 添加点位更新处理器
   */
  public addPointHandler(pointName: string, handler: (point: PointUpdate) => void): void {
    if (!this.pointHandlers.has(pointName)) {
      this.pointHandlers.set(pointName, []);
    }
    this.pointHandlers.get(pointName)?.push(handler);
  }

  /**
   * 移除点位更新处理器
   */
  public removePointHandler(pointName: string, handler: (point: PointUpdate) => void): void {
    const handlers = this.pointHandlers.get(pointName);
    if (handlers) {
      this.pointHandlers.set(pointName, handlers.filter(h => h !== handler));
    }
  }

  /**
   * 处理点位更新
   */
  public handlePointUpdate(update: PointUpdate): void {
    const handlers = this.pointHandlers.get(update.name) || [];
    handlers.forEach(handler => {
      try {
        handler(update);
      } catch (error) {
        console.error('点位处理器错误:', error);
      }
    });
  }

  /**
   * 获取所有点位
   */
  public getAllPoints(): any[] {
    return this.allPoints;
  }

  /**
   * 获取加载状态
   */
  public isLoading(): boolean {
    return this.loading;
  }

  /**
   * 获取错误信息
   */
  public getError(): string {
    return this.error;
  }
}

// 导出单例实例
export const pointService = PointService.getInstance();
