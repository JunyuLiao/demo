# Deployment Guide

This guide shows how to deploy the High-Dimensional Regret Minimization Algorithm to a public website.

## 🚀 Railway Deployment (Recommended)

### Prerequisites
- GitHub account
- Railway account (free at [railway.app](https://railway.app))

### Steps

1. **Push code to GitHub**
   ```bash
   git add .
   git commit -m "Prepare for Railway deployment"
   git push origin main
   ```

2. **Deploy to Railway**
   - Go to [railway.app](https://railway.app)
   - Sign in with GitHub
   - Click "New Project"
   - Select "Deploy from GitHub repo"
   - Choose your repository
   - Railway will automatically detect it's a Python app

3. **Configure Environment**
   - Railway will automatically install dependencies from `requirements.txt`
   - The `build.sh` script will compile the C++ algorithm
   - The `Procfile` tells Railway how to start the app

4. **Access Your App**
   - Railway will provide a public URL (e.g., `https://your-app.railway.app`)
   - Your Flask app with real C++ algorithm will be live!

## 🔧 Alternative: Heroku Deployment

### Prerequisites
- GitHub account
- Heroku account (free at [heroku.com](https://heroku.com))
- Heroku CLI installed

### Steps

1. **Install Heroku CLI**
   ```bash
   # macOS
   brew install heroku/brew/heroku
   
   # Or download from https://devcenter.heroku.com/articles/heroku-cli
   ```

2. **Login to Heroku**
   ```bash
   heroku login
   ```

3. **Create Heroku App**
   ```bash
   heroku create your-app-name
   ```

4. **Deploy**
   ```bash
   git add .
   git commit -m "Deploy to Heroku"
   git push heroku main
   ```

## 🌐 Alternative: Render Deployment

1. Go to [render.com](https://render.com)
2. Sign in with GitHub
3. Create "New Web Service"
4. Connect your repository
5. Set build command: `chmod +x build.sh && ./build.sh`
6. Set start command: `python web_app_simple.py`

## 📋 What Gets Deployed

- ✅ **Flask backend** (Python)
- ✅ **C++ algorithm** (compiled binary)
- ✅ **Real-time processing** (not static)
- ✅ **User study features** (consent, feedback)
- ✅ **All datasets** (car.txt, etc.)

## 🔍 Troubleshooting

### Build Issues
- Check that `build.sh` is executable (`chmod +x build.sh`)
- Verify `requirements.txt` has all dependencies
- Ensure `Makefile` works in Linux environment

### Runtime Issues
- Check logs in Railway/Heroku dashboard
- Verify C++ binary is compiled correctly
- Ensure all file paths are correct

## 🎯 Result

After deployment, you'll have:
- **Public URL** accessible worldwide
- **Real C++ algorithm** running on server
- **Full functionality** (not just static demo)
- **User study** ready for participants

The deployed version will be identical to your localhost version!
