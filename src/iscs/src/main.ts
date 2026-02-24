import { createApp } from 'vue'
import { createPinia } from 'pinia'
import App from './App.vue'
import { wsClient } from './services/websocket'
import { usePointStore } from './stores/pointStore'
import http from './services/http'

const app = createApp(App)
const pinia = createPinia()
app.use(pinia)
// global binding: subscribe to default points and fetch initial values
const store = usePointStore()
const deviceIds = ['fan.1','pump.1','light.1']

// subscribe via websocket
deviceIds.forEach(id => wsClient.subscribePoint(id))

// handle POINT_UPDATE messages globally
const handler = (m: any) => {
	store.updatePoint({ name: m.name, value: m.value, timestamp: m.timestamp, driver: m.driver, device: m.device });
};
wsClient.addHandler(handler);

// initial batch query to populate values
(async ()=>{
	try{
		const resp = await http.post('/v1/points/batch', { pointIds: deviceIds })
		if (resp && resp.data && Array.isArray(resp.data.points)){
			const pts = resp.data.points
			for (let i = 0; i < pts.length && i < deviceIds.length; ++i){
				const v = pts[i]
				store.updatePoint({ name: deviceIds[i], value: v.value ?? '', timestamp: v.ts ?? Date.now(), driver: '', device: '' })
			}
		}
	}catch(e){ console.warn('batch query failed', e) }
})()

app.mount('#app')
