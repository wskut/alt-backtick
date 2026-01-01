#include "KeyboardMonitor.h"

const char* GetKeyName(DWORD vkCode)
{
    switch (vkCode)
    {
        // Control keys
        case VK_LCONTROL: return "Left Ctrl";
        case VK_RCONTROL: return "Right Ctrl";
        case VK_CONTROL: return "Ctrl";
        case VK_LSHIFT: return "Left Shift";
        case VK_RSHIFT: return "Right Shift";
        case VK_SHIFT: return "Shift";
        case VK_LMENU: return "Left Alt";
        case VK_RMENU: return "Right Alt";
        case VK_MENU: return "Alt";
        
        // Special keys
        case VK_CANCEL: return "Cancel";
        case VK_BACK: return "Backspace";
        case VK_TAB: return "Tab";
        case VK_RETURN: return "Enter";
        case VK_CAPITAL: return "Caps Lock";
        case VK_ESCAPE: return "Escape";
        case VK_SPACE: return "Space";
        case VK_PRIOR: return "Page Up";
        case VK_NEXT: return "Page Down";
        case VK_END: return "End";
        case VK_HOME: return "Home";
        case VK_LEFT: return "Left Arrow";
        case VK_UP: return "Up Arrow";
        case VK_RIGHT: return "Right Arrow";
        case VK_DOWN: return "Down Arrow";
        case VK_SNAPSHOT: return "Print Screen";
        case VK_SCROLL: return "Scroll Lock";
        case VK_PAUSE: return "Pause";
        case VK_INSERT: return "Insert";
        case VK_DELETE: return "Delete";
        
        // number keys
        case '0': return "0";
        case '1': return "1";
        case '2': return "2";
        case '3': return "3";
        case '4': return "4";
        case '5': return "5";
        case '6': return "6";
        case '7': return "7";
        case '8': return "8";
        case '9': return "9";
        
        // alphabet keys
        case 'A': return "A";
        case 'B': return "B";
        case 'C': return "C";
        case 'D': return "D";
        case 'E': return "E";
        case 'F': return "F";
        case 'G': return "G";
        case 'H': return "H";
        case 'I': return "I";
        case 'J': return "J";
        case 'K': return "K";
        case 'L': return "L";
        case 'M': return "M";
        case 'N': return "N";
        case 'O': return "O";
        case 'P': return "P";
        case 'Q': return "Q";
        case 'R': return "R";
        case 'S': return "S";
        case 'T': return "T";
        case 'U': return "U";
        case 'V': return "V";
        case 'W': return "W";
        case 'X': return "X";
        case 'Y': return "Y";
        case 'Z': return "Z";
        
        // Function keys
        case VK_F1: return "F1";
        case VK_F2: return "F2";
        case VK_F3: return "F3";
        case VK_F4: return "F4";
        case VK_F5: return "F5";
        case VK_F6: return "F6";
        case VK_F7: return "F7";
        case VK_F8: return "F8";
        case VK_F9: return "F9";
        case VK_F10: return "F10";
        case VK_F11: return "F11";
        case VK_F12: return "F12";
        
        // Numpad keys
        case VK_NUMLOCK: return "Num Lock";
        case VK_MULTIPLY: return "Numpad *";
        case VK_ADD: return "Numpad +";
        case VK_SEPARATOR: return "Numpad Separator";
        case VK_SUBTRACT: return "Numpad -";
        case VK_DECIMAL: return "Numpad .";
        case VK_DIVIDE: return "Numpad /";
        case VK_NUMPAD0: return "Numpad 0";
        case VK_NUMPAD1: return "Numpad 1";
        case VK_NUMPAD2: return "Numpad 2";
        case VK_NUMPAD3: return "Numpad 3";
        case VK_NUMPAD4: return "Numpad 4";
        case VK_NUMPAD5: return "Numpad 5";
        case VK_NUMPAD6: return "Numpad 6";
        case VK_NUMPAD7: return "Numpad 7";
        case VK_NUMPAD8: return "Numpad 8";
        case VK_NUMPAD9: return "Numpad 9";
        
        // Windows keys
        case VK_LWIN: return "Left Windows";
        case VK_RWIN: return "Right Windows";
        case VK_APPS: return "Application (Context Menu)";
        
        // Korean keyboard specific keys
        case VK_HANGUL: return "Hangul (한/영)";
        case VK_HANJA: return "Hanja (한자)";
        
        // OEM keys
        case VK_OEM_PLUS: return "[OEM_+] =";
        case VK_OEM_COMMA: return "[OEM_,] ,";
        case VK_OEM_MINUS: return "[OEM_-] -";
        case VK_OEM_PERIOD: return "[OEM_.] .";
        case VK_OEM_1: return "[OEM_1] ;";
        case VK_OEM_2: return "[OEM_2] /";
        case VK_OEM_3: return "[OEM_3] `";
        case VK_OEM_4: return "[OEM_4] [";
        case VK_OEM_5: return "[OEM_5] \\";
        case VK_OEM_6: return "[OEM_6] ]";
        case VK_OEM_7: return "[OEM_7] '";
        
        default:
            return "Unknown";
    }
}
