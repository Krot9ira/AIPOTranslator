# AIPOTranslator

**AIPOTranslator** is a tool for translating `.PO` files using AI models.  
It has been tested with the `qwen3:30b` model running locally through [Ollama](https://ollama.com) on a system with:

- GPU: RTX 3080 Ti (12 GB VRAM)  
- RAM: 48 GB  

---

## 🛠️ Building the Project with CMake and Visual Studio 2022

To generate the project files, follow these steps:

```
# Open Command Prompt and run:
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
```

---

## ⚙️ Configuration

You can change translation settings in `POTranslator.h`:

- `apiUrl` — Default is `localhost`. Change it to your server's URL if needed.
- `model` — Default is `qwen3:30b`. You can replace this with any compatible model name.

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
- You can use any model compatible with your setup, not just `qwen3:30b`.

---

Thanks for checking out the tool! If it’s useful to you, a wishlist for my game would be hugely appreciated: https://store.steampowered.com/app/3890110?utm_source=github
