#include "ButtonManager.h"

/**
 * @brief 
 * @see https://gist.github.com/lovyan03/d68efb62fefdb74e31f965b1387c29fe
 */

void ButtonManager::Setup()
{
    memset(changed_millis, 0, BTN_STACK_MAX * sizeof(std::uint32_t));

#if defined( ARDUINO_M5Stack_Core_ESP32 ) || defined( ARDUINO_M5STACK_FIRE ) // M5Stack
    gpios[static_cast<int>(ButtonType::A)] = 39;
    gpios[static_cast<int>(ButtonType::B)] = 38;
    gpios[static_cast<int>(ButtonType::C)] = 37;
#elif  defined( ARDUINO_M5Stick_C_Plus ) || defined( ARDUINO_M5Stick_C ) // M5Stick C
    gpios[static_cast<int>(ButtonType::A)] = 37;
    gpios[static_cast<int>(ButtonType::B)] = 39;
    gpios[static_cast<int>(ButtonType::C)] = -1;
#endif

    for (auto pin : gpios)
    {
        if(pin >= 0)
        {
            pinMode(pin, 0x01); 
        }
    }
}


// std::uint32_t ButtonManager::Check(void)
// {
//     std::uint32_t hitres = 0;
//     std::uint_fast8_t hitlen = 0;

//     // 登録されているコマンドと一致するものを探すループ
//     for (const auto& cmd : commands)
//     {
//         // 既にヒットしたものより短いものは除外
//         if (hitlen >= cmd.len) continue;

//         // 履歴と一致しないものは除外
//         if (memcmp(_btns_stack, cmd.inputs, sizeof(bits_btn_t) * cmd.len)) continue;

//         // ヒットしたものを記録しておく
//         hitres = cmd.value;
//         hitlen = cmd.len;
//     }
//     return hitres;
// }


/**
 * @brief Loop内でCall.
 *          同じ状態がdebounce期間続いたら、ボタン状態が変化したと認識させている。
 * 
 * @return true 状態に変化アリ
 * @return false 状態変化ナシ
 */
bool ButtonManager::Update(void)
{
    std::uint32_t   now         = millis(); // 現在のミリ秒時間を取得
    std::uint32_t   btns        = 0;        // ボタンのGPIOの状態をbtnsにビット列として取得
    constexpr int   num_buttons = static_cast<std::underlying_type<ButtonType>::type>(ButtonType::Num_Buttons);
    const auto btns_after_debounce_old = btns_after_debounce;

    ////
    /// 現在のボタン状態を取得
    //
    for (int i = 0; i < num_buttons; ++i) 
    {
        if (gpios[i] >= 0 && !digitalRead(gpios[i]))
        {
            btns |= 1 << i;
        } 
    }
    
    ////
    /// 記録しているボタンの状態と比較
    /// 前回と状態に変化があったら、時間を記録し一旦終了(変化無し)
    // 
    for (int i = 0; i < num_buttons; ++i) 
    {
        if(((btns >> i) & 0x1) != ((btns_before_debounce >> i) & 0x1))
        {
            if(((btns >> i) & 0x1))
            {
                btns_before_debounce |= 1 << i;
            }else
            {
                btns_before_debounce &= ~(1 << i);
                btns_longpressed     &= ~(1 << i);
            }
            changed_millis[i] = now;
        }
    }
    
    ////
    /// 入力変化後、デバウンス処理時間が経過していることを確認
    //
    btns_changed = 0;
    for (int i = 0; i < num_buttons; ++i) 
    {
        if(((btns_before_debounce >> i) & 0x1) != ((btns_after_debounce >> i) & 0x1) && now - changed_millis[i] >= debounce_millis)
        {
            btns_changed |= (1 << i);

            if(((btns >> i) & 0x1))
            {
                btns_after_debounce |= 1 << i;
            }else
            {
                btns_after_debounce &= ~(1 << i);
            }
        }
    }
    
    prev_millis = now;

    return (btns_after_debounce_old != btns_after_debounce);

/*
    if (btns_after_debounce != btns && now >= changed_millis + debounce_millis)
    {
        // デバウンス処理後の入力状態を更新
        btns_after_debounce = btns;

        // 履歴を一つ追加
        memmove(&btns_stack[1], btns_stack, (BTN_STACK_MAX - 1) * sizeof(bits_btn_t));
        btns_stack[0] = btns_after_debounce;

        // 履歴とコマンドの比較処理を行う
        btn_value = Check();
        
        return true;
    }

    // 入力無変化での長押し時間経過を確認（無操作での時間経過時も含む）
    if (btns_stack[1] != btns && now >= changed_millis + hold_millis)
    {
        // 履歴を一つ追加（長押し時間が経過した場合、履歴の今回値と前回値が同じになる）
        memmove(&btns_stack[1], btns_stack, (BTN_STACK_MAX - 1) * sizeof(bits_btn_t));

        // 履歴とコマンドの比較処理を行う
        btn_value = Check();
        
        return true;
    }

    // 無入力状態が長押し時間２回分経過した場合は履歴をクリア
    if (!btns && msec - changed_millis > (hold_millis<<1))
    {
        memset(btns_stack, 0, BTN_STACK_MAX * sizeof(bits_btn_t));
        changed_millis = msec;
        btn_value = 0;

        return true;
    }
    return false;    
*/
}

/*
    // 0x01 -> long press(1s)
	// 0x02 -> short press
	if (M5.Axp.GetBtnPress() == 0x02)

*/

bool ButtonManager::IsDown(ButtonType type)
{
    if(type == ButtonType::Num_Buttons)
    {
        return false;
    }

    return ((btns_after_debounce >> static_cast<int>(type)) & 0x1);
}

bool ButtonManager::IsPressed(ButtonType type)
{
#if defined( AXPBUTTON_ENABLE ) // M5StickC
    if(type == ButtonType::C)
    {
        return M5.Axp.GetBtnPress() == 0x02;
    }
#endif      // AXPBUTTON_ENABLE

    if(!IsDown(type))
    {
        return false;
    }

    return ((btns_changed >> static_cast<int>(type)) & 0x1);
}

bool ButtonManager::IsLongPressed(ButtonType type)
{
#if defined( AXPBUTTON_ENABLE ) // M5StickC
    if(type == ButtonType::C)
    {
        return M5.Axp.GetBtnPress() == 0x01;
    }
#endif      // AXPBUTTON_ENABLE

    if(!IsDown(type))
    {
        return false;
    }
    
    std::uint32_t   now = millis(); // 現在のミリ秒時間を取得
    const auto      id  = static_cast<int>(type);
    
    if((now - changed_millis[id] < hold_millis))
    {
        return false;
    }
    if((btns_longpressed >> id)&1)
    {
        return false;
    }
    
    btns_longpressed |= 1 << id;

    return true;
}

