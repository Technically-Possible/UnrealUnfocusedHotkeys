# Unfocused Hotkeys

An Unreal Engine plugin that fires a Blueprint event when a global hotkey is pressed, even while your game is not focused.

Originally made for the YouTuber NeoCranium, now released for everyone to use.

> Work in progress. Windows (Win64) only. A full setup guide and prebuilt downloads are coming soon.

## Using it from source

Clone this repository into `YourProject/Plugins/UnfocusedHotkeys`, then open the project and let Unreal rebuild the module. This needs Visual Studio with the **Game development with C++** workload.

In any Blueprint, get the **Background Key Subsystem**, bind **On Hot Key Pressed**, then call **Set Global Hot Key** with a key and modifiers.

## License

[MIT](LICENSE) © 2026 Jenn.zip
