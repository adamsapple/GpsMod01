#ifndef __SIMPLEBUTTON_H__
#define __SIMPLEBUTTON_H__

// #if defined(BOARD_M5STACK)
//     #include <M5Stack.h>
// #elif defined(BOARD_M5STICKC)
//     #include <M5StickC.h>
// #endif

#if defined( ARDUINO_M5Stack_Core_ESP32 ) || defined( ARDUINO_M5STACK_FIRE ) // M5Stack
	#include <M5Stack.h>
#elif defined(ARDUINO_M5Stick_C_Plus)		// M5StickC Plusの場合
	#include <M5StickCPlus.h>
	#define AXPBUTTON_ENABLE
#elif defined( ARDUINO_M5Stick_C ) // M5Stick C
	#include <M5StickC.h>
	#define AXPBUTTON_ENABLE
#endif

// ボタン状態のビット列を保持する型（ボタン数を９個以上に増やす場合は必要に応じてuint16_tやuint32_tに変更する）
typedef std::uint8_t bits_btn_t;

enum class ButtonType
{ 
	A,
	B,
	C,
	Num_Buttons
};

enum : bits_btn_t
{
    bit_a = 1<<static_cast<int>(ButtonType::A),
    bit_b = 1<<static_cast<int>(ButtonType::B),
    bit_c = 1<<static_cast<int>(ButtonType::C)
};

struct command_holder
{
	const std::uint32_t     value;    // ボタンコマンドヒット時の識別値
	const std::uint_fast8_t len;      // ボタンコマンド長
	const bits_btn_t        *inputs;  // ボタンコマンド配列
};

static constexpr std::size_t BTN_STACK_MAX = 8; // ボタン状態の入力履歴保持数 （コマンド列以上の値にすること）

class ButtonManager
{
private:
	std::uint32_t   btn_value;                  	// コマンド判定でヒットした値
	std::int32_t    gpios[static_cast<std::underlying_type<ButtonType>::type>(ButtonType::Num_Buttons)];       		// ボタンのGPIOリスト
	std::uint32_t   changed_millis[static_cast<std::underlying_type<ButtonType>::type>(ButtonType::Num_Buttons)];   // 入力変化時点の時間記録
	std::uint32_t	prev_millis       	 = 0;		// 前回の時間
	bits_btn_t      btns_before_debounce = 0;       // デバウンス処理前のボタン状態記録
	bits_btn_t      btns_after_debounce  = 0;       // デバウンス処理後のボタン状態記録
	bits_btn_t		btns_changed		 = 0;      	// debounce後、変化が確定したボタン
	bits_btn_t		btns_longpressed	 = 0;      	// 
	bits_btn_t      btns_stack[BTN_STACK_MAX];		// ボタン状態の履歴

public:
	const std::uint32_t debounce_millis   = 300;	// デバウンス処理の待ち時間
	const std::uint32_t hold_millis       = 800;	// 長押し判定の待ち時間
	
	void Setup();
	bool Update();
	bool IsDown(ButtonType);
	bool IsPressed(ButtonType);
	bool IsLongPressed(ButtonType);
	//std::uint32_t Check();
};

#endif // __SIMPLEBUTTON_H__
