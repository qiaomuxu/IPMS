const app = getApp();
const API_BASE = 'http://rc5e6a3f.natappfree.cc/api';

Page({
  data: {
    records: [],
    currentTab: 'all',
    loading: false,
    plateColorMap: {
      '蓝': 'blue',
      '黑': 'black',
      '白': 'white',
      '黄': 'yellow',
      '绿': 'green'
    }
  },

  onLoad: function() {
    this.loadRecords();
  },

  onShow: function() {
    this.loadRecords();
  },

  loadRecords: function() {
    this.setData({ loading: true });
    const url = this.data.currentTab === 'all'
      ? `${API_BASE}/vehicle/records`
      : `${API_BASE}/vehicle/records/in`;

    wx.request({
      url: url,
      method: 'GET',
      success: (res) => {
        if (res.data && res.data.length > 0) {
          this.setData({ records: res.data });
        } else {
          this.setData({ records: [] });
        }
      },
      fail: (err) => {
        console.error('请求失败:', err);
        wx.showToast({
          title: '网络请求失败',
          icon: 'none'
        });
      },
      complete: () => {
        this.setData({ loading: false });
      }
    });
  },

  switchTab: function(e) {
    const tab = e.currentTarget.dataset.tab;
    this.setData({ currentTab: tab });
    this.loadRecords();
  },

  refreshRecords: function() {
    this.loadRecords();
    wx.showToast({
      title: '刷新中',
      icon: 'loading'
    });
  },

  viewDetail: function(e) {
    const id = e.currentTarget.dataset.id;
    wx.navigateTo({
      url: `/pages/detail/detail?id=${id}`
    });
  },

  openGate: function() {
    wx.request({
      url: `${API_BASE}/images/gate/open`,
      method: 'POST',
      success: (res) => {
        if (res.data.success) {
          wx.showToast({
            title: '抬杆命令已发送',
            icon: 'success'
          });
        } else {
          wx.showToast({
            title: res.data.message || '发送失败',
            icon: 'none'
          });
        }
      },
      fail: (err) => {
        console.error('请求失败:', err);
        wx.showToast({
          title: '网络请求失败',
          icon: 'none'
        });
      }
    });
  },

  closeGate: function() {
    wx.request({
      url: `${API_BASE}/images/gate/close`,
      method: 'POST',
      success: (res) => {
        if (res.data.success) {
          wx.showToast({
            title: '落杆命令已发送',
            icon: 'success'
          });
        } else {
          wx.showToast({
            title: res.data.message || '发送失败',
            icon: 'none'
          });
        }
      },
      fail: (err) => {
        console.error('请求失败:', err);
        wx.showToast({
          title: '网络请求失败',
          icon: 'none'
        });
      }
    });
  }
});