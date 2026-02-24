ISCS Monitor Prototype

Run locally:

1. cd src/iscs
2. npm install
3. npm run dev

By default the prototype connects to WebSocket `ws://localhost:9000` and proxies `/api` to `http://localhost:8080` (change in `vite.config.ts`).

- `src/services/websocket.ts` implements a small reconnecting WebSocket client. It sends `SUBSCRIBE <pointId>` messages to the server and expects `POINT_UPDATE` JSON messages.
- `src/stores/pointStore.ts` is a Pinia store holding point values.
- `src/views/TestView.vue` is a simple demo showing three devices and buttons to send control commands.
