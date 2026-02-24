<template>
  <div class="user-list">
    <div class="header">
      <h3>用户管理</h3>
      <button class="btn btn-primary" @click="openAddUserModal">添加用户</button>
    </div>
    
    <table class="table">
      <thead>
        <tr>
          <th>ID</th>
          <th>用户名</th>
          <th>角色</th>
          <th>状态</th>
          <th>创建时间</th>
          <th>操作</th>
        </tr>
      </thead>
      <tbody>
        <tr v-for="user in users" :key="user.id">
          <td>{{ user.id }}</td>
          <td>{{ user.username }}</td>
          <td>{{ user.role_cname }}</td>
          <td>{{ user.enable ? '启用' : '禁用' }}</td>
          <td>{{ formatTime(user.created_at) }}</td>
          <td>
            <button class="btn btn-sm btn-edit" @click="openEditUserModal(user)">编辑</button>
            <button class="btn btn-sm btn-delete" @click="confirmDeleteUser(user)">删除</button>
          </td>
        </tr>
      </tbody>
    </table>
    
    <!-- 添加/编辑用户模态框 -->
    <div class="modal" v-if="showUserModal">
      <div class="modal-content">
        <div class="modal-header">
          <h4>{{ editMode ? '编辑用户' : '添加用户' }}</h4>
          <button class="close" @click="showUserModal = false">&times;</button>
        </div>
        <div class="modal-body">
          <form @submit.prevent="saveUser">
            <div class="form-group">
              <label for="username">用户名</label>
              <input type="text" id="username" v-model="currentUser.username" required>
            </div>
            <div class="form-group">
              <label for="password">密码</label>
              <input type="password" id="password" v-model="currentUser.password" required>
              <small>密码必须≥8位，含大小写+数字</small>
            </div>
            <div class="form-group">
              <label for="role">角色</label>
              <select id="role" v-model="currentUser.role_id" required>
                <option value="">请选择角色</option>
                <option v-for="role in roles" :key="role.id" :value="role.id">
                  {{ role.cname }}
                </option>
              </select>
            </div>
            <div class="form-group">
              <label for="enable">状态</label>
              <input type="checkbox" id="enable" v-model="currentUser.enable">
              <span>启用</span>
            </div>
            <div class="modal-footer">
              <button type="button" class="btn btn-secondary" @click="showUserModal = false">取消</button>
              <button type="submit" class="btn btn-primary">{{ editMode ? '保存' : '添加' }}</button>
            </div>
          </form>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { userApi } from '../../services/userApi';

export default {
  name: 'UserList',
  data() {
    return {
      users: [],
      roles: [],
      showUserModal: false,
      editMode: false,
      currentUser: {
        id: '',
        username: '',
        password: '',
        role_id: '',
        role_name: '',
        role_cname: '',
        enable: true
      }
    };
  },
  mounted() {
    this.fetchUsers();
    this.fetchRoles();
  },
  methods: {
    async fetchUsers() {
      try {
        const data = await userApi.getUsers();
        // service may return paged dto or array; handle both
        this.users = data.items || data || [];
      } catch (error) {
        console.error('获取用户列表失败:', error);
      }
    },
    async fetchRoles() {
      try {
        const data = await userApi.getRoles();
        this.roles = data || [];
      } catch (error) {
        console.error('获取角色列表失败:', error);
      }
    },
    openAddUserModal() {
      this.editMode = false;
      this.currentUser = {
        id: '',
        username: '',
        password: '',
        role_id: '',
        role_name: '',
        role_cname: '',
        enable: true
      };
      this.showUserModal = true;
    },
    openEditUserModal(user) {
      this.editMode = true;
      this.currentUser = {
        id: user.id,
        username: user.username,
        password: '', // 编辑时密码为空，用户可以选择不修改
        role_id: user.role_id,
        role_name: user.role_name,
        role_cname: user.role_cname,
        enable: user.enable
      };
      this.showUserModal = true;
    },
    async saveUser() {
      try {
        if (!this.validatePassword(this.currentUser.password)) {
          alert('密码必须≥8位，含大小写+数字');
          return;
        }

        if (this.editMode) {
          // 编辑用户
          const userToUpdate = { ...this.currentUser };
          if (!userToUpdate.password) {
            delete userToUpdate.password; // 如果密码为空则不更新密码
          }
          await userApi.updateUser(this.currentUser.id, userToUpdate);
        } else {
          const {id, ...userData} = this.currentUser; // 创建时不需要id
          // 添加用户
          await userApi.createUser(userData);
        }

        this.showUserModal = false;
        this.fetchUsers();
      } catch (error) {
        console.error('保存用户失败:', error);
      }
    },
    confirmDeleteUser(user) {
      if (confirm(`确定要删除用户 ${user.username} 吗？`)) {
        this.deleteUser(user.id);
      }
    },
    async deleteUser(userId) {
      try {
        await userApi.deleteUser(userId);
        this.fetchUsers();
      } catch (error) {
        console.error('删除用户失败:', error);
      }
    },
    validatePassword(password) {
      if (!password) return false;
      if (password.length < 8) return false;
      if (!/[A-Z]/.test(password)) return false;
      if (!/[a-z]/.test(password)) return false;
      if (!/[0-9]/.test(password)) return false;
      return true;
    },
    formatTime(timestamp) {
      const date = new Date(timestamp);
      return date.toLocaleString();
    }
  }
};
</script>

<style scoped>
.user-list {
  padding: 20px;
}

.header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
}

.table {
  width: 100%;
  border-collapse: collapse;
}

.table th,
.table td {
  border: 1px solid #ddd;
  padding: 8px;
  text-align: left;
}

.table th {
  background-color: #f2f2f2;
}

.btn {
  padding: 6px 12px;
  border: none;
  border-radius: 4px;
  cursor: pointer;
}

.btn-primary {
  background-color: #007bff;
  color: white;
}

.btn-edit {
  background-color: #28a745;
  color: white;
  margin-right: 5px;
}

.btn-delete {
  background-color: #dc3545;
  color: white;
}

/* 模态框样式 */
.modal {
  position: fixed;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  background-color: rgba(0, 0, 0, 0.5);
  display: flex;
  justify-content: center;
  align-items: center;
}

.modal-content {
  background-color: white;
  padding: 20px;
  border-radius: 8px;
  width: 400px;
  max-width: 90%;
}

.modal-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
}

.close {
  background: none;
  border: none;
  font-size: 24px;
  cursor: pointer;
}

.form-group {
  margin-bottom: 15px;
}

.form-group label {
  display: block;
  margin-bottom: 5px;
}

.form-group input,
.form-group select {
  width: 100%;
  padding: 8px;
  border: 1px solid #ddd;
  border-radius: 4px;
}

.modal-footer {
  display: flex;
  justify-content: flex-end;
  margin-top: 20px;
}

.btn-secondary {
  background-color: #6c757d;
  color: white;
  margin-right: 10px;
}
</style>
