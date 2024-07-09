// food_data.h  
#ifndef FOOD_DATA_H  
#define FOOD_DATA_H  
 
typedef struct {    
    char name[50];      // 食材名称  
    int quantity;       // 食材数量  
    int period;         // 食材常规保质期（假设以天为单位）    
    char storageDate[20];// 食材存储日期（可以是一个日期字符串）    
} FoodData;  
 
// typedef struct {    
//     char name[50];      // 食物名称  
//     int days;       // 食物过期天数（或剩余天数）  
// } expiredFoodData;  

#endif // FOOD_DATA_H
