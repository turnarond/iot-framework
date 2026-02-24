// 统一 API 客户端
// 优先使用 Vite 提供的环境变量 VITE_API_BASE，若未设置则回退到相对路径 `/api`
const API_BASE: string = import.meta.env.VITE_API_BASE || '/api';

function buildUrl(path: string): string {
  // path 可能以 '/' 开头或不以 '/' 开头，确保拼接正确
  if (path.startsWith('/')) return `${API_BASE}${path}`;
  return `${API_BASE}/${path}`;
}

async function request<T = any>(path: string, options: RequestInit = {}): Promise<T> {
  const url = buildUrl(path);
  try {
    const resp = await fetch(url, {
      ...options,
      headers: {
        'Content-Type': 'application/json',
        ...options.headers
      }
    });

    if (!resp.ok) {
      const text = await resp.text().catch(() => '');
      const err = new Error(`HTTP ${resp.status} ${resp.statusText} ${text}`);
      (err as any).status = resp.status;
      throw err;
    }

    if (resp.status === 204) return null as T;
    const contentType = resp.headers.get('content-type') || '';
    if (contentType.includes('application/json')) return await resp.json() as T;
    return await resp.text() as T;
  } catch (e) {
    // 在控制台打印更友好的信息，便于调试
    console.error('[apiClient] request error', { url, options, error: e });
    throw e;
  }
}

export { API_BASE, request };