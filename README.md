## 📋 Overview  
This is a minimal aim assist implementation written in pure C. It uses low-level mouse hook to slow down your mouse when a target appears in your crosshair, helping you stay accurate and avoid missing shots.

## 💻 How SetWindowsHookEx Works in Valorant  
Returning 1 after hooking the mouse will block it. However since Vanguard also hooks this function, it calls CallNextHookEx() after calling the function, preventing us from being able to return anything ourselves. 
However Vanguard doesn’t immediately call CallNextHookEx(), it first calls our function, giving us the opportunity to Sleep() during this time, which will freeze all mouse movement inputs.
The downside to this is that if the sleep amount is set too high, it could potentially freeze other mouse inputs, such as clicks as well.

### ⚙️ Usage
1. **Compile the Project**  
   - Install Visual Studio.  
   - Open the provided .sln file.  
   - Click on **Build** and select **Build Solution**.  

2. **Run the Program and Configure Valorant Settings**  
   - Run the generated .exe file and launch Valorant.  
   - Set the display mode to **Fullscreen Windowed** in Valorant's settings.  
   - Ensure the cheat's target color matches the enemy color you have selected in Valorant.  

**UC thread:**  
`https://www.unknowncheats.me/forum/valorant/675938-aim-assist-pure.html`
