#include "lvgl.h"

// 字体声明
extern lv_font_t font_chinese_24;

/* 全局控件 */
static lv_obj_t *label_temp;
static lv_obj_t *label_humi;
static lv_obj_t *label_status;
static lv_obj_t *label_uptime;
static lv_obj_t *bar_temp;
static lv_obj_t *bar_humi;
static lv_obj_t *btn_test;  // 按钮句柄



lv_obj_t *page_main_create(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);			//NULL 创建独立屏幕
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xF8F9FA), 0);

    // ====================== 关键：禁止页面滚动 ======================
lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);   // 禁止滚动
lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF); // 隐藏滚动条
    // ===============================================================

    // 主标题 纯中文
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "边缘网关监控");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_style_text_color(title, lv_color_black(), 0);
		lv_obj_set_style_text_font(title, &font_chinese_24, 0);					//给 title 这个标签设置显示字体并显示中文汉字

    // 温度卡片
		lv_obj_t *card_temp = lv_obj_create(scr);					//以主界面为父对象 创建基本部件
    lv_obj_set_size(card_temp, 130, 90);
    lv_obj_align(card_temp, LV_ALIGN_LEFT_MID, -15, 10);
    lv_obj_set_style_bg_color(card_temp, lv_color_hex(0x16213E), 0);
    lv_obj_set_style_border_width(card_temp, 0, 0);										//去掉边框、不显示边框线。
    lv_obj_set_style_radius(card_temp, 8, 0);													//设置圆角半径为 8

		//温度文本
    lv_obj_t *icon_temp = lv_label_create(card_temp);
    lv_label_set_text(icon_temp, "温度");
    lv_obj_align(icon_temp, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_style_text_color(icon_temp, lv_color_hex(0xFF6B6B), 0);
    lv_obj_set_style_text_font(icon_temp, &font_chinese_24, 0);								//给这个标签设置显示字体并显示中文汉字

		//符号文本
    label_temp = lv_label_create(card_temp);
    lv_label_set_text(label_temp, "--.-C");
    lv_obj_align(label_temp, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_set_style_text_color(label_temp, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_temp, &font_chinese_24, 0);							//给这个标签设置显示字体并显示中文汉字

    // 温度条
    bar_temp = lv_bar_create(card_temp);											//以温度卡片为父对象 创建滑动条
    lv_obj_set_size(bar_temp, 100, 8);
    lv_obj_align(bar_temp, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_bar_set_range(bar_temp, -10, 60);
    lv_bar_set_value(bar_temp, 0, LV_ANIM_ON);										//初始值
    lv_obj_set_style_bg_color(bar_temp, lv_color_hex(0xFF6B6B), LV_PART_INDICATOR);

    // 湿度卡片
    lv_obj_t *card_humi = lv_obj_create(scr);
    lv_obj_set_size(card_humi, 130, 90);
    lv_obj_align(card_humi, LV_ALIGN_RIGHT_MID, 15, 10);
    lv_obj_set_style_bg_color(card_humi, lv_color_hex(0x16213E), 0);
    lv_obj_set_style_border_width(card_humi, 0, 0);					//无边框
    lv_obj_set_style_radius(card_humi, 8, 0);									//圆角


		//湿度文本
    lv_obj_t *icon_humi = lv_label_create(card_humi);
    lv_label_set_text(icon_humi, "湿度");
    lv_obj_align(icon_humi, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_style_text_color(icon_humi, lv_color_hex(0x4ECDC4), 0);
    lv_obj_set_style_text_font(icon_humi, &font_chinese_24, 0);

    label_humi = lv_label_create(card_humi);
    lv_label_set_text(label_humi, "--.-%");
    lv_obj_align(label_humi, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_set_style_text_color(label_humi, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_humi, &font_chinese_24, 0);

    // 湿度条
    bar_humi = lv_bar_create(card_humi);
    lv_obj_set_size(bar_humi, 100, 8);
    lv_obj_align(bar_humi, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_bar_set_range(bar_humi, 0, 100);
    lv_bar_set_value(bar_humi, 0, LV_ANIM_ON);
    lv_obj_set_style_bg_color(bar_humi, lv_color_hex(0x4ECDC4), LV_PART_INDICATOR);



		// 底部状态栏 —— 铺满全屏 + 禁止滚动 + 完美固定
		lv_obj_t *footer = lv_obj_create(scr);								//以主界面为父对象 创建基本部件
		lv_obj_set_width(footer, lv_obj_get_width(scr));  // 铺满屏幕宽度
		lv_obj_set_height(footer, 40);
		lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, 0); // 贴死底部
		lv_obj_set_style_bg_color(footer, lv_color_hex(0x0F3460), 0);
		lv_obj_set_style_radius(footer, 0, 0);           // 直角更好看（铺满必须直角）
		lv_obj_set_style_border_width(footer, 0, 0);     // 去掉边框

		// 关键：禁止 footer 自己滚动
		lv_obj_clear_flag(footer, LV_OBJ_FLAG_SCROLLABLE);
		lv_obj_set_scrollbar_mode(footer, LV_SCROLLBAR_MODE_OFF);


		//以底部部件为父对象 创建文本
    label_status = lv_label_create(footer);											//创建成功返回地址 不为空NULL
    lv_label_set_text(label_status, "● 等待连接...");
    lv_obj_align(label_status, LV_ALIGN_LEFT_MID, 8, 0);
    lv_obj_set_style_text_color(label_status, lv_color_hex(0xA0A0A0), 0);
    lv_obj_set_style_text_font(label_status, &font_chinese_24, 0);

		//以底部部件为父对象 创建文本
    label_uptime = lv_label_create(footer);
    lv_label_set_text(label_uptime, "运行 00:00:00");
    lv_obj_align(label_uptime, LV_ALIGN_RIGHT_MID, -8, 0);
    lv_obj_set_style_text_color(label_uptime, lv_color_hex(0x808080), 0);
    lv_obj_set_style_text_font(label_uptime, &font_chinese_24, 0);

    return scr;
}

// 更新温湿度
void page_main_update_data(float temp, float humi)
{
		static float temp_last=-999;												//静态变量只会初始化一次 魔法数 让第一数据不相等  默认为0
		static float humi_last=-999;
		if(temp!=temp_last || humi!=humi_last)
		{
			
			lv_label_set_text_fmt(label_temp, "%.1fC", temp);					
			lv_label_set_text_fmt(label_humi, "%.1f%%", humi);				
			lv_bar_set_value(bar_temp, (int)temp, LV_ANIM_ON);				
			lv_bar_set_value(bar_humi, (int)humi, LV_ANIM_ON);				
			
			temp_last=temp;
			humi_last=humi;
		}
		

}

// 更新连接状态
void page_main_update_status(uint8_t connected)
{
	if(!label_status) return;																									//创建成功为1 label_status 底部部件文本
    if(connected)
    {
        lv_label_set_text(label_status, "在线");
        lv_obj_set_style_text_color(label_status, lv_color_hex(0x4ECDC4), 0);
    }
    else
    {
        lv_label_set_text(label_status, "离线");
        lv_obj_set_style_text_color(label_status, lv_color_hex(0xFF6B6B), 0);
    }
}

// 更新运行时间
void page_main_update_uptime(uint32_t seconds)
{
    if(!label_uptime) return;
    uint32_t h = seconds / 3600;
    uint32_t m = (seconds % 3600) / 60;
    uint32_t s = seconds % 60;
    lv_label_set_text_fmt(label_uptime, "运行 %02d:%02d:%02d", h, m, s);
}


