import { defineStore } from 'pinia'
import { reactive } from 'vue'

export type PointValue = {
  name: string;
  value: string;
  timestamp: number;
  driver?: string;
  device?: string;
}

export const usePointStore = defineStore('points', ()=>{
  const points = reactive<Record<string, PointValue>>({});
  const pending: Record<string, number> = reactive({});

  function updatePoint(p: PointValue){
    points[p.name] = p;
    // clear pending if any for this point
    if (pending[p.name]) {
      delete pending[p.name];
    }
  }

  function getPoint(name: string){
    return points[name];
  }

  function setPending(name: string){
    pending[name] = Date.now();
  }

  function isPending(name: string){
    return !!pending[name];
  }

  function clearPending(name: string){
    if (pending[name]) delete pending[name];
  }
  return { points, updatePoint, getPoint, setPending, isPending, clearPending }
})
