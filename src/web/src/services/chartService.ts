import * as echarts from 'echarts';

/**
 * 图表服务
 * 处理图表的初始化、更新和数据管理
 */
export class ChartService {
  private charts: Map<string, echarts.ECharts> = new Map();
  private dataHistory: Map<string, Array<{ timestamp: number; value: number }>> = new Map();

  /**
   * 初始化图表
   */
  public initChart(container: HTMLElement, chartId: string): void {
    if (this.charts.has(chartId)) {
      this.charts.get(chartId)?.dispose();
    }
    const chart = echarts.init(container);
    this.charts.set(chartId, chart);
  }

  /**
   * 更新图表
   */
  public updateChart(
    chartId: string,
    selectedPoints: string[],
    chartType: string,
    timeRange: number
  ): void {
    const chart = this.charts.get(chartId);
    if (!chart || selectedPoints.length === 0) return;

    const now = Date.now();
    const startTime = now - timeRange;

    // 准备系列数据
    const series = selectedPoints.map((pointName, index) => {
      const history = this.dataHistory.get(pointName) || [];
      const filteredData = history.filter(item => item.timestamp >= startTime);
      
      return {
        name: pointName,
        data: filteredData.map(item => [item.timestamp, parseFloat(item.value) || 0]),
        type: chartType === 'area' ? 'line' : chartType,
        smooth: true,
        symbol: chartType === 'scatter' ? 'circle' : 'none',
        sampling: 'average',
        itemStyle: {
          color: this.getColor(index)
        },
        areaStyle: chartType === 'area' ? {
          color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
            {
              offset: 0,
              color: this.getColor(index, 0.5)
            },
            {
              offset: 1,
              color: this.getColor(index, 0.1)
            }
          ])
        } : undefined
      };
    });

    const option = {
      tooltip: {
        trigger: 'axis',
        formatter: function(params: any) {
          let result = ChartService.formatTime(params[0].value[0]) + '<br/>';
          params.forEach((item: any) => {
            result += `${item.marker}${item.seriesName}: ${item.value[1]}<br/>`;
          });
          return result;
        }
      },
      legend: {
        data: selectedPoints,
        bottom: 0
      },
      xAxis: {
        type: 'time',
        boundaryGap: false
      },
      yAxis: {
        type: 'value',
        name: '数值'
      },
      grid: {
        left: '3%',
        right: '4%',
        bottom: '10%',
        top: '3%',
        containLabel: true
      },
      series
    };

    chart.setOption(option);
  }

  /**
   * 添加数据点
   */
  public addDataPoint(pointName: string, timestamp: number, value: number): void {
    if (!this.dataHistory.has(pointName)) {
      this.dataHistory.set(pointName, []);
    }
    const history = this.dataHistory.get(pointName)!;
    history.push({ timestamp, value });
    
    // 保持历史记录不超过1000个点
    if (history.length > 1000) {
      history.shift();
    }
  }

  /**
   * 清除数据历史
   */
  public clearDataHistory(pointName?: string): void {
    if (pointName) {
      this.dataHistory.delete(pointName);
    } else {
      this.dataHistory.clear();
    }
  }

  /**
   * 调整图表大小
   */
  public resizeChart(chartId: string): void {
    this.charts.get(chartId)?.resize();
  }

  /**
   * 销毁图表
   */
  public disposeChart(chartId: string): void {
    const chart = this.charts.get(chartId);
    if (chart) {
      chart.dispose();
      this.charts.delete(chartId);
    }
  }

  /**
   * 获取颜色
   */
  private getColor(index: number, alpha: number = 1): string {
    const colors = [
      `rgba(64, 158, 255, ${alpha})`,
      `rgba(103, 194, 58, ${alpha})`,
      `rgba(230, 162, 60, ${alpha})`,
      `rgba(245, 108, 108, ${alpha})`,
      `rgba(194, 129, 249, ${alpha})`,
      `rgba(72, 187, 120, ${alpha})`
    ];
    return colors[index % colors.length];
  }

  /**
   * 格式化时间
   */
  public static formatTime(timestamp: number): string {
    const date = new Date(timestamp);
    return date.toLocaleString();
  }
}

// 导出单例实例
export const chartService = new ChartService();
