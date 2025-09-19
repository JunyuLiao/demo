# High-Dimensional Regret Minimization - User Study

A web-based user study for evaluating interactive decision-making algorithms in high-dimensional preference learning.

## 🌐 Live Demo

The study is deployed on GitHub Pages: **[https://JunyuLiao.github.io/High-dimensional-Regret-Minimization-Demo/](https://JunyuLiao.github.io/High-dimensional-Regret-Minimization-Demo/)**

## 📋 Study Overview

This user study evaluates the effectiveness of our interactive algorithm for finding preferred options from high-dimensional data. Participants interact with a simulated algorithm that presents car options with various attributes and learns their preferences through a series of comparison questions.

### Study Flow
1. **Consent Page** - Study introduction and participant agreement
2. **Interactive Algorithm** - Answer comparison questions about car options
3. **Feedback Collection** - Rate the algorithm performance (1-10 scale)
4. **Thank You** - Study completion and appreciation

## 🚀 Deployment Instructions

### Prerequisites
- GitHub account
- Repository with GitHub Pages enabled

### Steps to Deploy

1. **Fork or Clone this Repository**
   ```bash
   git clone https://github.com/JunyuLiao/High-dimensional-Regret-Minimization-Demo.git
   cd High-dimensional-Regret-Minimization-Demo
   ```

2. **Enable GitHub Pages**
   - Go to repository Settings
   - Navigate to Pages section
   - Set Source to "GitHub Actions"

3. **Update Repository URL**
   - Edit `static/index.html`, `static/feedback.html`, and `static/thank_you.html`
   - Replace `yourusername` with your actual GitHub username in all links

4. **Push to Main Branch**
   ```bash
   git add .
   git commit -m "Deploy to GitHub Pages"
   git push origin main
   ```

5. **Verify Deployment**
   - GitHub Actions will automatically deploy the site
   - Visit `https://JunyuLiao.github.io/High-dimensional-Regret-Minimization-Demo/` to see your live study

## 📁 File Structure

```
High-dimensional-Regret-Minimization-Demo/
├── static/                    # Static files for GitHub Pages
│   ├── index.html            # Main study page
│   ├── feedback.html         # Feedback collection page
│   └── thank_you.html        # Thank you page
├── .github/
│   └── workflows/
│       └── deploy.yml        # GitHub Actions deployment
├── templates/                # Original Flask templates
├── web_app_simple.py         # Original Flask backend
└── README_GITHUB_PAGES.md    # This file
```

## 🔧 Technical Details

### Static Implementation
- **Frontend**: Pure HTML, CSS, and JavaScript
- **Data Storage**: LocalStorage for feedback collection
- **Algorithm Simulation**: Pre-defined demo questions and responses
- **Responsive Design**: Mobile-friendly interface

### Demo Data
The static version includes:
- 3 pre-defined comparison questions
- 2 final recommendation options
- Realistic car attribute data (MPG, production, etc.)

### Data Collection
- **Study Progress**: Tracked in sessionStorage
- **User Feedback**: Stored in localStorage
- **Anonymous**: No personal information collected

## 📊 Data Analysis

### Accessing Collected Data
1. Open browser Developer Tools (F12)
2. Go to Application/Storage tab
3. View Local Storage for `userFeedback` entries

### Data Format
```json
[
  {
    "rating": 8,
    "timestamp": "2024-01-15T10:30:45.123Z",
    "studyData": {
      "startTime": "2024-01-15T10:15:30.456Z",
      "endTime": "2024-01-15T10:30:45.123Z",
      "questionsAnswered": 12,
      "completed": true
    }
  }
]
```

## 🎯 Study Features

### Interactive Elements
- **Click-to-Select**: Click option cards to make choices
- **Keyboard Input**: Type 1, 2, or 0 to respond
- **Horizontal Scrolling**: View all attributes on mobile
- **Early Stop**: Stop answering and get recommendations

### User Experience
- **Auto-Start**: Algorithm begins automatically after consent
- **Progress Tracking**: Visual counter for questions answered
- **Loading States**: Clear feedback during processing
- **Responsive Design**: Works on all device sizes

## 🔒 Privacy & Ethics

- **Anonymous Participation**: No personal data collected
- **Local Storage Only**: Data stays in user's browser
- **Clear Consent**: Participants understand study purpose
- **Voluntary**: Users can stop at any time

## 🛠️ Customization

### Modifying Demo Data
Edit the `demoQuestions` and `finalRecommendations` arrays in `static/index.html`:

```javascript
const demoQuestions = [
  {
    options: [
      {
        id: 1,
        dimensions: {
          "Attribute 1": 100.000,
          "Attribute 2": 50.000
        }
      }
    ]
  }
];
```

### Styling Changes
Modify CSS in the `<style>` section of each HTML file to match your branding.

### Adding More Questions
Extend the `demoQuestions` array with additional comparison scenarios.

## 📈 Analytics (Optional)

To track study participation, you can add Google Analytics:

1. Add tracking code to each HTML file
2. Monitor page views and user interactions
3. Analyze completion rates and feedback scores

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## 📄 License

This project is for research purposes. Please ensure compliance with your institution's research ethics guidelines.

## 📞 Support

For questions about the study or technical issues:
- Create an issue in this repository
- Contact the research team

---

**Note**: This is a static version designed for GitHub Pages. For the full interactive version with C++ backend, use the Flask application in the main repository.
