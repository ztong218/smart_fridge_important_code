//reminder.js
const app = getApp()
Page({
  data:{
    productId: app.globalData.productId,
    deviceName: app.globalData.deviceName,
    stateReported: {},
    expiredFood: [],
    soonExpireFood: [],
  },
  onLoad: function (options) {
    console.log("index onLoad")
    if (!app.globalData.productId) {
      wx.showToast({
        title: "产品ID不能为空",
        icon: 'none',
        duration: 3000
      })
      return
    } else if (!app.globalData.deviceName) {
      wx.showToast({
        title: "设备名称不能为空",
        icon: 'none',
        duration: 3000
      })
      return
    }
    this.update();
  },
  
  //事件处理函数
  update() {
    wx.showLoading()
    wx.cloud.callFunction({
      name: 'iothub-shadow-query',
      data: {
        ProductId: app.globalData.productId,
        DeviceName: app.globalData.deviceName,
        SecretId: app.globalData.secretId,
        SecretKey: app.globalData.secretKey,
      },
      success: res => {
        wx.showToast({
          icon: 'none',
          title: 'Subscribe完成，获取云端数据成功',
        })
        let deviceData = JSON.parse(res.result.Data)

        this.setData({
          stateReported: deviceData.payload.state.reported
        })
        // 调用displayFood方法来处理和分类食物数据
        if (Array.isArray(this.data.stateReported.food)) {
          this.displayFood();
        }
        console.log("result:", deviceData)
      },
      fail: err => {
        wx.showToast({
          icon: 'none',
          title: 'Subscribe失败，获取云端数据失败',
        })
        console.error('[云函数] [iotexplorer] 调用失败：', err)
      }
    })
  },

  displayFood: function() {
    const food = this.data.stateReported.food;
    const expired = []; // 已经过期的食物数组
    const soonExpire = []; // 即将过期的食物数组

    food.forEach(item => {
      if (item.period <= 0) {
        expired.push(item);
      } else if (item.period > 0 && item.period < 3) {
        soonExpire.push(item);
      }
    });

    // 将分类后的食物数组设置到页面数据中
    this.setData({
      expiredFood: expired,
      soonExpireFood: soonExpire
    });
  },

    /**
   * 生命周期函数--监听页面显示
   */
  onShow: function () {
  
  },
  /**
   * 生命周期函数--监听页面卸载
   */
  onUnload: function () {

  },
    /**
   * 页面相关事件处理函数--监听用户下拉动作
   */
  onPullDownRefresh: function () {

  },
  
  /**
   * 页面上拉触底事件的处理函数
   */
  onReachBottom: function () {

  },
})