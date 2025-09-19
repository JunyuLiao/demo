#!/bin/bash

# GitHub Pages Setup Script for JunyuLiao
# This script helps set up the repository for GitHub Pages deployment

echo "🚀 Setting up GitHub Pages deployment for JunyuLiao..."
echo ""

# Check if git is initialized
if [ ! -d ".git" ]; then
    echo "📁 Initializing git repository..."
    git init
fi

# Add remote repository
echo "🔗 Adding remote repository..."
git remote add origin https://github.com/JunyuLiao/highdim-rm-user-study.git 2>/dev/null || echo "Remote already exists"

# Add all files
echo "📦 Adding files to git..."
git add .

# Commit files
echo "💾 Committing files..."
git commit -m "Initial commit: User study for GitHub Pages deployment

- Static HTML pages for GitHub Pages
- Interactive algorithm simulation
- Feedback collection system
- Responsive design for all devices
- Automated deployment with GitHub Actions"

# Push to GitHub
echo "🚀 Pushing to GitHub..."
git push -u origin main

echo ""
echo "✅ Setup complete!"
echo ""
echo "🌐 Next steps:"
echo "1. Go to https://github.com/JunyuLiao/highdim-rm-user-study"
echo "2. Go to Settings → Pages"
echo "3. Set Source to 'GitHub Actions'"
echo "4. Wait 2-3 minutes for deployment"
echo "5. Visit: https://JunyuLiao.github.io/highdim-rm-user-study/"
echo ""
echo "📊 Your user study will be live at:"
echo "   https://JunyuLiao.github.io/highdim-rm-user-study/"
