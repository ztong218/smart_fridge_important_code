//menu.js
const app = getApp()
Page({
  data:{
    productId: app.globalData.productId,
    deviceName: app.globalData.deviceName,
    stateReported: {},
    showRecipes: {
      carrotPotato: false,
      appleBanana: false,
      lemonPotato: false,
      daikonChiken: false,
      eggplantPepper: false
    },
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
    this.update()
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
        // 调用checkFoodsAndShowRecipes方法来根据食物信息选择菜谱
        if (Array.isArray(this.data.stateReported.food)) {
          this.checkFoodsAndShowRecipes();
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

  checkFoodsAndShowRecipes: function() {
    const { food } = this.data.stateReported;
    const requiredFoodsCarrotPotato = ['土豆', '胡萝卜'];
    const requiredFoodsAppleBanana = ['苹果', '香蕉'];
    const requiredFoodsLemonPotato = ['柠檬', '土豆'];
    const requiredFoodsDaikonChiken = ['白萝卜'];
    const requiredFoodsEggplantPepper = ['茄子', '青椒'];

    // 检查是否有所需的食材
    // const hasCarrotPotato = true;
    const hasCarrotPotato = requiredFoodsCarrotPotato.some(ingredient =>
      food.some(f => f.name === ingredient && f.storageData > 0 && f.period > 0)
    );

    const hasAppleBanana = true;
    // const hasAppleBanana = requiredFoodsAppleBanana.some(ingredient =>
    //   food.some(f => f.name === ingredient && f.storageData > 0 && f.period > 0)
    // );

    // const hasLemonPotato = true;
    const hasLemonPotato = requiredFoodsLemonPotato.some(ingredient =>
      food.some(f => f.name === ingredient && f.storageData > 0 && f.period > 0)
    );

    // const hasDaikonChiken = true;
    const hasDaikonChiken = requiredFoodsDaikonChiken.some(ingredient =>
      food.some(f => f.name === ingredient && f.storageData > 0 && f.period > 0)
    );

    const hasEggplantPepper = true;
    // const hasEggplantPepper = requiredFoodsEggplantPepper.some(ingredient =>
    //   food.some(f => f.name === ingredient && f.storageData > 0 && f.period > 0)
    // );

    // 更新菜谱展示状态
    this.setData({
      showRecipes: {
        carrotPotato: hasCarrotPotato,
        appleBanana: hasAppleBanana,
        lemonPotato: hasLemonPotato,
        laikonChiken: hasDaikonChiken,
        eggplantPepper: hasEggplantPepper
      }
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