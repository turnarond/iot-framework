/**
 * 点位表单相关的自定义 Hook
 * 处理点位表单的逻辑，包括表单验证、数据处理等
 */
import { ref, reactive, computed } from 'vue';
import { TRANSFER_METHODS, VALIDATION_RULES } from '../constants/index';

// 点位表单类型定义
interface PointForm {
  id: string;
  name: string;
  address: string;
  device_id: string;
  datatype: number;
  point_type: number;
  transfer_type: number;
  linear_raw_min: number;
  linear_raw_max: number;
  linear_eng_min: number;
  linear_eng_max: number;
  advanced_algo_lib: string;
  advanced_param1: string;
  advanced_param2: string;
  advanced_param3: string;
  advanced_param4: string;
  enable_control: boolean;
  enable_history: boolean;
  poll_rate: number;
  description: string;
  [key: string]: any;
}

// 错误类型定义
interface FormErrors {
  [key: string]: string;
}

/**
 * 点位表单 Hook
 * @param {Boolean} editMode - 是否为编辑模式
 * @param {Object} pointData - 点位数据
 * @returns {Object} - 包含表单状态和方法的对象
 */
export function usePointForm(editMode: boolean = false, pointData: PointForm | null = null) {
  // 表单数据
  const pointForm = reactive<PointForm>({
    id: '',
    name: '',
    address: '',
    device_id: '',
    datatype: 2,           // 默认整型
    point_type: 1,         // 默认设备变量
    transfer_type: 0,      // 默认None
    linear_raw_min: 0,
    linear_raw_max: 100,
    linear_eng_min: 0,
    linear_eng_max: 100,
    advanced_algo_lib: '',
    advanced_param1: '',
    advanced_param2: '',
    advanced_param3: '',
    advanced_param4: '',
    enable_control: false,
    enable_history: true,
    poll_rate: 1,
    description: ''
  });

  // 表单验证错误
  const errors = ref<FormErrors>({});

  // 初始化表单数据
  if (editMode && pointData) {
    Object.assign(pointForm, pointData);
  }

  // 计算属性：当前传输方法
  const currentTransferMethod = computed(() => {
    return pointForm.transfer_type;
  });

  // 计算属性：是否显示线性缩放配置
  const showLinearConfig = computed(() => {
    return currentTransferMethod.value === TRANSFER_METHODS.LINEAR.value;
  });

  // 计算属性：是否显示高级算法配置
  const showAdvancedConfig = computed(() => {
    return currentTransferMethod.value === TRANSFER_METHODS.ADVANCED.value;
  });

  /**
   * 验证表单
   * @returns {Boolean} - 是否验证通过
   */
  const validateForm = (): boolean => {
    errors.value = {};
    let isValid = true;

    // 验证必填字段
    if (!pointForm.name) {
      errors.value.name = '点位名称不能为空';
      isValid = false;
    }

    if (!pointForm.address) {
      errors.value.address = '点位地址不能为空';
      isValid = false;
    }

    if (!pointForm.device_id) {
      errors.value.device_id = '请选择设备';
      isValid = false;
    }

    if (!pointForm.datatype) {
      errors.value.datatype = '请选择数据类型';
      isValid = false;
    }

    if (!pointForm.point_type) {
      errors.value.point_type = '请选择点位类型';
      isValid = false;
    }

    if (!pointForm.poll_rate || pointForm.poll_rate < VALIDATION_RULES.POLL_RATE_MIN) {
      errors.value.poll_rate = `上报频率必须大于等于${VALIDATION_RULES.POLL_RATE_MIN}`;
      isValid = false;
    }

    // 验证线性缩放
    if (showLinearConfig.value) {
      if (pointForm.linear_raw_min >= pointForm.linear_raw_max) {
        errors.value.linear_raw_min = '原始最小值必须小于原始最大值';
        isValid = false;
      }

      if (pointForm.linear_eng_min >= pointForm.linear_eng_max) {
        errors.value.linear_eng_min = '工程最小值必须小于工程最大值';
        isValid = false;
      }
    }

    return isValid;
  };

  /**
   * 处理表单提交
   * @returns {Object|null} - 处理后的表单数据，验证失败返回null
   */
  const handleSubmit = (): PointForm | null => {
    if (!validateForm()) {
      return null;
    }

    // 处理表单数据
    const formData = { ...pointForm };

    // 清理无关字段
    if (formData.transfer_type !== TRANSFER_METHODS.LINEAR.value) {
      delete formData.linear_raw_min;
      delete formData.linear_raw_max;
      delete formData.linear_eng_min;
      delete formData.linear_eng_max;
    }

    if (formData.transfer_type !== TRANSFER_METHODS.ADVANCED.value) {
      delete formData.advanced_algo_lib;
      delete formData.advanced_param1;
      delete formData.advanced_param2;
      delete formData.advanced_param3;
      delete formData.advanced_param4;
    }

    // 确保数字类型
    formData.device_id = Number(formData.device_id);
    formData.datatype = Number(formData.datatype);
    formData.point_type = Number(formData.point_type);
    formData.transfer_type = Number(formData.transfer_type);
    formData.poll_rate = Number(formData.poll_rate);

    if (showLinearConfig.value) {
      formData.linear_raw_min = Number(formData.linear_raw_min);
      formData.linear_raw_max = Number(formData.linear_raw_max);
      formData.linear_eng_min = Number(formData.linear_eng_min);
      formData.linear_eng_max = Number(formData.linear_eng_max);
    }

    return formData;
  };

  /**
   * 重置表单
   */
  const resetForm = () => {
    Object.assign(pointForm, {
      id: '',
      name: '',
      address: '',
      device_id: '',
      datatype: 2,
      point_type: 1,
      transfer_type: 0,
      linear_raw_min: 0,
      linear_raw_max: 100,
      linear_eng_min: 0,
      linear_eng_max: 100,
      advanced_algo_lib: '',
      advanced_param1: '',
      advanced_param2: '',
      advanced_param3: '',
      advanced_param4: '',
      enable_control: false,
      enable_history: true,
      poll_rate: 1,
      description: ''
    });
    errors.value = {};
  };

  return {
    pointForm,
    errors,
    currentTransferMethod,
    showLinearConfig,
    showAdvancedConfig,
    validateForm,
    handleSubmit,
    resetForm
  };
}