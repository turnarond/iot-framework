import { createRouter, createWebHistory } from 'vue-router'
import Monitor from '@/views/Monitor.vue'
import DriverView from '@/views/DriverView.vue'
import DeviceView from '@/views/DeviceView.vue'
import PointView from '@/views/PointView.vue'
import AlarmView from '@/views/AlarmView.vue'
import LinkageView from '@/views/LinkageView.vue'
import SystemView from '@/views/SystemView.vue'
import VideoView from '@/views/VideoView.vue'
import ServerMgrView from '@/views/ServerMgrView.vue'
import LinkageTriggerView from '@/views/linkage/TriggerView.vue'
import LinkageActionView from '@/views/linkage/ActionView.vue'
import LinkageRuleView from '@/views/linkage/RuleView.vue'

const routes = [
  {
    path: '/',
    redirect: '/servermgr'
  },
  {
    path: '/servermgr',
    name: 'ServerMgr',
    component: ServerMgrView,
    meta: {
      title: '基本信息'
    }
  },
  {
    path: '/monitor',
    name: 'Monitor',
    component: Monitor,
    meta: {
      title: '状态监控'
    }
  },
  {
    path: '/driver',
    name: 'Driver',
    component: DriverView,
    meta: {
      title: '驱动管理'
    }
  },
  {
    path: '/device',
    name: 'Device',
    component: DeviceView,
    meta: {
      title: '设备管理'
    }
  },
  {
    path: '/point',
    name: 'Point',
    component: PointView,
    meta: {
      title: '点位管理'
    }
  },
  {
    path: '/alarm',
    name: 'Alarm',
    component: AlarmView,
    meta: {
      title: '报警管理'
    }
  },
  {
    path: '/linkage',
    name: 'Linkage',
    redirect: '/linkage/rule',
    meta: {
      title: '联动管理'
    },
    children: [
      {
        path: 'trigger',
        name: 'LinkageTrigger',
        component: LinkageTriggerView,
        meta: {
          title: '触发器管理'
        }
      },
      {
        path: 'action',
        name: 'LinkageAction',
        component: LinkageActionView,
        meta: {
          title: '执行器管理'
        }
      },
      {
        path: 'rule',
        name: 'LinkageRule',
        component: LinkageRuleView,
        meta: {
          title: '联动规则'
        }
      }
    ]
  },
  {
    path: '/system',
    name: 'System',
    component: SystemView,
    meta: {
      title: '系统配置'
    }
  },
  {
    path: '/video',
    name: 'Video',
    component: VideoView,
    meta: {
      title: '视频监控'
    }
  }
]

const router = createRouter({
  history: createWebHistory(),
  routes
})

router.beforeEach((to, from, next) => {
  // 设置页面标题
  if (to.meta.title) {
    document.title = `${to.meta.title} - 综合监控系统`
  } else {
    document.title = '综合监控系统'
  }
  next()
})

export default router
