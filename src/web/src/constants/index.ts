/**
 * 常量管理文件
 * 集中管理所有常量，提高代码可维护性和可读性
 */

// 数据类型
export const DATATYPES = [
  { value: 1, label: '布尔型' },
  { value: 2, label: '整型' },
  { value: 3, label: '浮点型' },
  { value: 4, label: '字符串型' }
];

// 数据类型映射
export const DATATYPE_MAP = {
  1: '布尔型',
  2: '整型',
  3: '浮点型',
  4: '字符串型'
};

// 点位类型
export const POINT_TYPES = [
  { value: 1, label: '设备变量' },
  { value: 2, label: '计算变量' }
];

// 点位类型映射
export const POINT_TYPE_MAP = {
  1: '设备变量',
  2: '计算变量'
};

// 传输方法
export const TRANSFER_METHODS = {
  NONE: { value: 0, label: 'None' },
  LINEAR: { value: 1, label: 'LinearScaling' },
  ADVANCED: { value: 2, label: 'AdvancedAlgorithm' }
};

// 传输方法映射
export const TRANSFER_METHOD_MAP = {
  0: 'None',
  1: 'LinearScaling',
  2: 'AdvancedAlgorithm'
};

// 模块类型
export const MODULES = {
  DRIVER: 'driver',
  DEVICE: 'device',
  POINT: 'point',
  ALARM: 'alarm',
  LINKAGE: 'linkage'
};

// 模块标题映射
export const MODULE_TITLE_MAP = {
  [MODULES.DRIVER]: '驱动管理',
  [MODULES.DEVICE]: '设备管理',
  [MODULES.POINT]: '点位管理',
  [MODULES.ALARM]: '报警管理',
  [MODULES.LINKAGE]: '联动管理'
};

// 默认分页配置
export const DEFAULT_PAGE_SIZE = 10;
export const DEFAULT_PAGE = 1;

// 表单验证规则
export const VALIDATION_RULES = {
  POLL_RATE_MIN: 1
};

// 报警方法
export const ALARM_METHODS = {
  HIGH_HIGH: { value: 1, label: '高高限', shortLabel: 'HH' },
  HIGH: { value: 2, label: '高限', shortLabel: 'H' },
  LOW: { value: 3, label: '低限', shortLabel: 'L' },
  LOW_LOW: { value: 4, label: '低低限', shortLabel: 'LL' },
  FIXED_VALUE: { value: 5, label: '定值', shortLabel: 'FX' }
};

// 报警方法列表（用于下拉框）
export const ALARM_METHODS_LIST = [
  { value: 1, label: '高高限 (HH)' },
  { value: 2, label: '高限 (H)' },
  { value: 3, label: '低限 (L)' },
  { value: 4, label: '低低限 (LL)' },
  { value: 5, label: '定值 (FX)' }
];

// 报警方法映射
export const ALARM_METHOD_MAP = {
  1: '高高限',
  2: '高限',
  3: '低限',
  4: '低低限',
  5: '定值'
};

// 报警方法短标签映射
export const ALARM_METHOD_SHORT_MAP = {
  1: 'HH',
  2: 'H',
  3: 'L',
  4: 'LL',
  5: 'FX'
};