const API_BASE = 'http://rc5e6a3f.natappfree.cc/api';

Page({
  data: {
    record: null,
    loading: true,
    imageUrl: '',
    plateColorMap: {
      '蓝': 'blue',
      '黑': 'black',
      '白': 'white',
      '黄': 'yellow',
      '绿': 'green'
    }
  },

  onLoad: function(options) {
    const id = options.id;
    if (id) {
      this.loadRecordDetail(id);
    }
  },

  loadRecordDetail: function(id) {
    wx.request({
      url: `${API_BASE}/vehicle/records/${id}`,
      method: 'GET',
      success: (res) => {
        if (res.statusCode === 200 && res.data) {
          // 提取文件名，处理完整路径和仅文件名两种情况
          let imagePath = res.data.imagePath || '';
          if (imagePath) {
            // 如果是完整路径，提取最后一部分文件名
            if (imagePath.includes('\\') || imagePath.includes('/')) {
              imagePath = imagePath.split(/[\\/]/).pop();
            }
          }
          this.setData({
            record: res.data,
            imageUrl: `http://rc5e6a3f.natappfree.cc/uploads/${imagePath}`
          });
        } else {
          wx.showToast({
            title: '记录不存在',
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
      },
      complete: () => {
        this.setData({ loading: false });
      }
    });
  },

  previewImage: function() {
    if (this.data.imageUrl) {
      wx.previewImage({
        urls: [this.data.imageUrl],
        current: this.data.imageUrl
      });
    }
  }
});