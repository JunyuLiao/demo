# 🚀 GitHub Pages Deployment Guide for JunyuLiao

## Step 1: Create New Repository

1. **Go to GitHub.com** and sign in
2. **Click "+" → "New repository"**
3. **Repository settings:**
   - **Name**: `highdim-rm-user-study`
   - **Description**: "User study for high-dimensional regret minimization algorithm evaluation"
   - **Visibility**: Public ✅
   - **Initialize**: Leave all boxes unchecked ❌

## Step 2: Upload Files to New Repository

### Option A: Using GitHub Web Interface
1. **Go to your new repository**
2. **Click "uploading an existing file"**
3. **Drag and drop these files:**
   - `index.html`
   - `static/` folder (with all contents)
   - `.github/workflows/deploy.yml`
   - `README_GITHUB_PAGES.md`
   - `user_feedback.json` (if you want to include existing data)

### Option B: Using Git Commands
```bash
# Navigate to your current project folder
cd "/Users/junyuliao/Desktop/user study/highdim_rm"

# Initialize git (if not already done)
git init

# Add remote repository
git remote add origin https://github.com/JunyuLiao/High-dimensional-Regret-Minimization-Demo.git

# Add all files
git add .

# Commit files
git commit -m "Initial commit: User study for GitHub Pages"

# Push to GitHub
git push -u origin main
```

## Step 3: Enable GitHub Pages

1. **Go to repository Settings** (not repository code)
2. **Scroll down to "Pages" section** (left sidebar)
3. **Under "Source":**
   - Select "GitHub Actions" ✅
   - Click "Save"

## Step 4: Verify Deployment

1. **Wait 2-3 minutes** for GitHub Actions to complete
2. **Check Actions tab** in your repository to see deployment status
3. **Visit your live site:**
   - URL: `https://JunyuLiao.github.io/highdim-rm-user-study/`

## Step 5: Test Your Study

1. **Open the live URL** in a new browser tab
2. **Complete the full study flow:**
   - Click "Start Study"
   - Answer 3 comparison questions
   - Submit feedback rating
   - See thank you page
3. **Verify data collection:**
   - Open Developer Tools (F12)
   - Go to Application → Local Storage
   - Check for `userFeedback` entries

## 🔧 Troubleshooting

### If GitHub Actions Fails:
1. **Check the Actions tab** for error details
2. **Ensure all files are uploaded** correctly
3. **Verify `.github/workflows/deploy.yml`** exists

### If Pages Don't Load:
1. **Wait 5-10 minutes** for propagation
2. **Check repository Settings → Pages** for status
3. **Try incognito/private browsing** to clear cache

### If Study Doesn't Work:
1. **Open browser Developer Tools (F12)**
2. **Check Console tab** for JavaScript errors
3. **Verify all HTML files** are accessible

## 📊 Data Collection

### Accessing User Feedback:
1. **Open Developer Tools (F12)**
2. **Go to Application → Local Storage**
3. **Look for `userFeedback` key**
4. **Copy JSON data** for analysis

### Example Data Format:
```json
[
  {
    "rating": 8,
    "timestamp": "2024-01-15T10:30:45.123Z",
    "studyData": {
      "startTime": "2024-01-15T10:15:30.456Z",
      "endTime": "2024-01-15T10:30:45.123Z",
      "questionsAnswered": 3,
      "completed": true
    }
  }
]
```

## 🎯 Next Steps

1. **Share the URL** with study participants
2. **Monitor participation** through GitHub repository traffic
3. **Collect feedback data** from participants
4. **Analyze results** for your research

## 📞 Support

If you encounter any issues:
1. **Check GitHub Actions logs** in the Actions tab
2. **Verify file structure** matches the guide
3. **Test locally** by opening `index.html` in browser

---

**Your live study will be available at:**
# 🌐 https://JunyuLiao.github.io/High-dimensional-Regret-Minimization-Demo/
