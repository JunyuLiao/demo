# 🆓 Free Deployment on Render

This guide shows how to deploy your Flask app **completely free** on Render.

## ✅ Why Render?
- **100% FREE** (no credit card required)
- **750 hours/month** free hosting
- **Automatic deployment** from GitHub
- **Supports Python + C++** binaries
- **Easy setup** (5 minutes)

## 🚀 Step-by-Step Deployment

### Step 1: Push to GitHub
```bash
git add .
git commit -m "Ready for Render deployment"
git push origin main
```

### Step 2: Deploy on Render
1. **Go to [render.com](https://render.com)**
2. **Sign up** with GitHub (free)
3. **Click "New +"** → **"Web Service"**
4. **Connect your repository:**
   - Repository: `JunyuLiao/High-dimensional-Regret-Minization-Demo`
   - Branch: `main`
5. **Configure settings:**
   - **Name:** `highdim-rm-demo` (or any name you like)
   - **Environment:** `Python 3`
   - **Build Command:** `chmod +x build.sh && ./build.sh`
   - **Start Command:** `python web_app_simple.py`
6. **Click "Create Web Service"**

### Step 3: Wait for Deployment
- Render will automatically:
  - Install Python dependencies
  - Compile your C++ algorithm
  - Deploy your Flask app
- **Takes 5-10 minutes** for first deployment

### Step 4: Access Your App
- Render will give you a URL like: `https://highdim-rm-demo.onrender.com`
- **Your real algorithm is now live!**

## 🎯 What You Get

- ✅ **Public URL** accessible worldwide
- ✅ **Real C++ algorithm** running on server
- ✅ **Full Flask backend** (not static demo)
- ✅ **User study features** (consent, feedback)
- ✅ **Completely free** (no credit card needed)

## 🔧 Troubleshooting

### If Build Fails:
1. Check the build logs in Render dashboard
2. Ensure `build.sh` is executable
3. Verify all dependencies are in `requirements.txt`

### If App Doesn't Start:
1. Check the runtime logs
2. Verify `web_app_simple.py` runs locally
3. Check that C++ binary compiled successfully

## 📊 Free Tier Limits

- **750 hours/month** (enough for 24/7 uptime)
- **512MB RAM** (sufficient for your app)
- **1GB storage** (plenty for your files)
- **No credit card required**

## 🎉 Result

After deployment, you'll have a **public website** that works exactly like your localhost version, but accessible to anyone on the internet!

**No more static demo - this is the real algorithm running on a server!**
