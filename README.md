# AIPOTranslator
<img width="1280" height="666" alt="image" src="https://github.com/user-attachments/assets/96c06d85-b2cd-4f04-8f00-609ed2c4bbb4" />

**AIPOTranslator** is a tool for translating `.PO` files using AI models.  
It has been tested with the `qwen3.6:35b` model running locally through [Ollama](https://ollama.com) on a system with:

- GPU: RTX 3080 Ti (12 GB VRAM)  
- RAM: 48 GB  

---

## 🛠️ Building the Project with CMake and Visual Studio 2026

To run the project in vs2026, follow these steps:

```
Install vckpg https://learn.microsoft.com/ru-ru/vcpkg/get_started/get-started-vs?pivots=shell-powershell

```
Make file "CMakeUserPresets.json" where you can make your own cmake presets and importantly set path to your vcpkg

example of file "CMakeUserPresets.json"
{
    "version": 2,
    "configurePresets": [
        {
            "name": "default",
            "inherits": "vcpkg",
            "environment": {
                "VCPKG_ROOT": "E:/programming/vcpkg" //your path to vcpkg on your pc
            }
        }
    ]
}
```
# Open Command Prompt and run:
mkdir build
cmake --preset default
```
now you can open project folder in visual studio and it will work.
---

## ⚙️ Configuration

You can change translation settings:

- `apiUrl` — Default is `localhost`. Change it to your server's URL if needed.
- `model` — Default is `qwen3:30b-instruct`. You can replace this with any compatible model name.
- `propmpt` — Any "{language}" will be changed to name of folder in which file is located
- `Overwrite Original Files` - If true will overwrite original files, if false will create new file in same folder with postfix: _Translated

---

## 🧠 Using AI Models with Ollama

If you want to run models locally with **Ollama**:

1. Visit [https://ollama.com](https://ollama.com)
2. Follow the instructions to install and run a model locally
3. After your model is running (locally or remotely), and you've set `apiUrl` and `model` in the POTranslator.h, you can move on to the usage section

---

## 🚀 How to Use

1. **Build and run** the AIPOTranslator application
2. **Select a folder** containing subfolders with `.po` files  
   - Each subfolder's name should match the target language (e.g., `fr`, `es`, `de`)
3. **Wait for the translation process** to complete
4. **Check the output**:  
   - Translated files will be saved next to the originals with the following format:  
     `OriginalFileName_translated.po`

---

## 📁 Example Folder Structure

```
/translations
├── fr
│   └── messages.po
├── es
│   └── messages.po
```

In this case, `messages.po` in the `fr` folder will be translated to French, `es` to Spanish, and so on.

---

## 💬 Notes

- Make sure your model is running and accessible via the `apiUrl` before starting translation.
- You can use any model compatible with your setup, not just `qwen3.6:35b`.

---

Thanks for checking out the tool! If it’s useful to you, a wishlist for my game would be hugely appreciated: https://store.steampowered.com/app/3890110?utm_source=github
