# High-Dimensional Regret Minimization - User Study Demo

A web-based interactive demo for evaluating high-dimensional regret minimization algorithms through user preference learning.

## 🌐 Live Demo

**Try the interactive study:** [https://JunyuLiao.github.io/High-dimensional-Regret-Minimization-Demo/](https://JunyuLiao.github.io/High-dimensional-Regret-Minimization-Demo/)

## 📋 About This Demo

This interactive web application demonstrates our high-dimensional regret minimization algorithm through a user study. Participants interact with a simulated algorithm that learns their preferences by presenting comparison questions about car specifications.

### Key Features
- **Interactive Algorithm Simulation** - Experience how the algorithm learns preferences
- **Realistic Data** - Uses actual car specifications with multiple attributes
- **User Study Design** - Collects feedback on algorithm performance
- **Responsive Interface** - Works on desktop and mobile devices
- **Data Collection** - Anonymous feedback collection for research

## 🎯 Study Flow

1. **Consent & Introduction** - Learn about the study and provide consent
2. **Interactive Questions** - Answer comparison questions about car options
3. **Algorithm Learning** - Watch how the algorithm adapts to your preferences
4. **Final Recommendations** - Receive personalized car recommendations
5. **Feedback Collection** - Rate the algorithm's performance (1-10 scale)
6. **Study Completion** - Thank you message and study summary

## 🚀 Quick Start

### For Participants
1. Visit the [live demo](https://JunyuLiao.github.io/High-dimensional-Regret-Minimization-Demo/)
2. Read the study information and click "Agree and Start"
3. Answer the comparison questions by clicking your preferred option
4. Complete the feedback survey
5. View your personalized recommendations

### For Researchers
1. Clone this repository for local development
2. Follow the setup instructions below
3. Customize the demo data and questions
4. Deploy your own version

## 🛠️ Local Development

### Prerequisites
- Python 3.7+
- Flask
- Modern web browser

### Setup
```bash
# Clone the repository
git clone https://github.com/JunyuLiao/High-dimensional-Regret-Minimization-Demo.git
cd High-dimensional-Regret-Minimization-Demo

# Install Python dependencies
pip install -r requirements.txt

# Run the Flask application
python web_app_simple.py
```

### Static Version (GitHub Pages)
The repository includes a static version that works without a backend:
- `index.html` - Landing page
- `static/index.html` - Main interactive demo
- `static/feedback.html` - Feedback collection
- `static/thank_you.html` - Completion page

## 📊 Data Collection

### What Data is Collected
- **Study Progress** - Number of questions answered, completion time
- **User Preferences** - Choices made during the study
- **Algorithm Rating** - User's rating of algorithm performance (1-10)
- **Anonymous** - No personal information is collected

### Accessing Data
1. Open browser Developer Tools (F12)
2. Go to Application → Local Storage
3. Look for `userFeedback` entries
4. Export JSON data for analysis

## 🔧 Customization

### Modifying Demo Questions
Edit the `demoQuestions` array in `static/index.html`:

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

### Adding New Attributes
1. Update the `dimensions` object in demo questions
2. Modify the `map.json` file for attribute names
3. Adjust the CSS for proper display

### Styling Changes
Modify the CSS in the `<style>` section of each HTML file to match your branding.

## 📁 Repository Structure

```
High-dimensional-Regret-Minimization-Demo/
├── static/                          # Static files for GitHub Pages
│   ├── index.html                   # Main interactive demo
│   ├── feedback.html                # Feedback collection page
│   └── thank_you.html               # Thank you page
├── templates/                       # Flask templates
│   ├── consent.html                 # Study consent page
│   ├── index_simple.html            # Main study interface
│   ├── feedback.html                # Feedback page
│   └── thank_you.html               # Thank you page
├── .github/workflows/               # GitHub Actions
│   └── deploy.yml                   # Auto-deployment workflow
├── web_app_simple.py                # Flask backend
├── main_web_real.cpp                # C++ algorithm implementation
├── map.json                         # Attribute name mapping
├── user_feedback.json               # Collected feedback data
└── README.md                        # This file
```

## 🌐 Deployment

### GitHub Pages (Recommended)
1. Fork this repository
2. Enable GitHub Pages in repository settings
3. Set source to "GitHub Actions"
4. Your demo will be live at `https://yourusername.github.io/High-dimensional-Regret-Minimization-Demo/`

### Local Server
```bash
# Run Flask application
python web_app_simple.py

# Or serve static files
python -m http.server 8000
```

## 📈 Research Applications

This demo is designed for:
- **Algorithm Evaluation** - Test user satisfaction with interactive algorithms
- **User Experience Research** - Study how users interact with preference learning systems
- **Educational Purposes** - Demonstrate high-dimensional optimization concepts
- **A/B Testing** - Compare different algorithm approaches

## 🔒 Privacy & Ethics

- **Anonymous Participation** - No personal data collected
- **Local Storage Only** - Data stays in user's browser
- **Clear Consent** - Participants understand study purpose
- **Voluntary** - Users can stop at any time
- **Research Use** - Data used solely for algorithm evaluation

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## 📄 License

This project is for research and educational purposes. Please ensure compliance with your institution's research ethics guidelines.

## 📞 Support

For questions about the demo or technical issues:
- Create an issue in this repository
- Contact the research team

## 📚 Related Work

This demo is based on research in high-dimensional regret minimization and interactive preference learning. The algorithm learns user preferences through a series of comparison questions, significantly reducing the number of queries needed compared to traditional methods.

---

**Try the interactive demo:** [https://JunyuLiao.github.io/High-dimensional-Regret-Minimization-Demo/](https://JunyuLiao.github.io/High-dimensional-Regret-Minimization-Demo/)