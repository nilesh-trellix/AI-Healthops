# AI-Healthops

**AI-powered Health Monitoring and Resource Analysis for Trellix Endpoint Products**

AI-Healthops is a comprehensive system resource monitoring and analysis platform designed specifically for Trellix endpoint products. It combines real-time system monitoring with AI-powered analysis to provide intelligent insights and recommendations for optimal resource utilization and system health.

## 🌟 Features

### Core Monitoring Capabilities
- **Real-time Process Monitoring**: Track CPU usage, memory consumption, and I/O operations
- **System Resource Analysis**: Monitor kernel and user CPU usage, memory statistics, and disk I/O
- **Performance Metrics**: Detailed tracking of working set size, page faults, and pool usage
- **Visual Analytics**: Interactive charts and graphs for performance visualization

### AI-Powered Intelligence
- **Intelligent Analysis**: Leverages Google Vertex AI (Gemini 2.5-flash-lite) for smart resource analysis
- **Automated Recommendations**: AI-generated suggestions for system optimization
- **Alert Processing**: Real-time analysis of system alerts and anomalies
- **Predictive Insights**: Early warning system for potential resource issues

### Multi-Interface Support
- **Desktop Application**: Feature-rich Qt-based GUI for detailed monitoring
- **REST API**: RESTful endpoints for integration with existing systems
- **GraphQL API**: Flexible GraphQL interface for custom data queries
- **Template-based Prompts**: Jinja2 templates for consistent AI interactions

## 🏗️ Architecture

The system consists of two main components:

![Full System Architecture](DesignAndArchitecture/Full%20system%20Arch.PNG)

### 1. Desktop Monitoring Application (`AI-Healthops/`)
- **Technology Stack**: C++ with Qt framework
- **Real-time Monitoring**: Windows API integration for system metrics
- **Interactive UI**: Charts, tables, and dashboards for data visualization
- **Database Integration**: SQLite for historical data storage

### 2. AI Analysis API (`AI interface/`)
- **Technology Stack**: Python with FastAPI
- **AI Integration**: Google Vertex AI for intelligent analysis
- **Multiple Endpoints**: REST and GraphQL support
- **Template System**: Jinja2 for structured prompt generation

## 🚀 Quick Start

### Prerequisites
- **For Desktop App**: 
  - Qt 5.12 or higher
  - C++17 compatible compiler
  - Windows SDK
- **For AI API**:
  - Python 3.12+
  - Google Cloud Platform account
  - Vertex AI API enabled

### Installation

#### Desktop Application
```bash
cd AI-Healthops
qmake AI-Healthops_2.pro
make
```

#### AI API Service
```bash
cd "AI interface/trellix-wise"
python -m venv venv
venv\Scripts\activate
pip install -r requirements.txt

# Configure Google Cloud
gcloud auth application-default login
gcloud config set project YOUR_PROJECT_ID

# Start the service
python main.py
```

## 📊 Key Metrics Monitored

- **CPU Performance**: Kernel and user CPU percentages, total CPU time
- **Memory Usage**: Working set size, peak memory usage, page faults
- **I/O Operations**: Read/write IOPS, bytes per second, total bytes transferred
- **Process Details**: Page file usage, pool usage (paged/non-paged)
- **System Health**: Overall system performance and resource utilization

## 🔧 Configuration

### Environment Variables (AI API)
```env
GOOGLE_CLOUD_PROJECT=your-project-id
GOOGLE_CLOUD_LOCATION=us-central1
VERTEX_AI_MODEL=gemini-2.5-flash-lite
```

### Build Configuration (Desktop App)
- Modify `AI-Healthops_2.pro` for custom build settings
- Configure database path in `proc_database.cpp`
- Adjust monitoring intervals in application settings

## 📈 Use Cases

- **System Administrators**: Monitor Trellix endpoint health and performance
- **Security Teams**: Analyze resource usage patterns for anomaly detection
- **DevOps Engineers**: Integrate monitoring data with existing infrastructure
- **Performance Analysts**: Generate reports and insights for optimization

## 🛠️ Development

### Desktop Application Structure
```
AI-Healthops/
├── main.cpp              # Application entry point
├── mainwindow.cpp/.h     # Main UI and logic
├── proc_stats.cpp/.h     # Performance statistics collection
├── proc_database.cpp/.h  # Database operations
└── debug/release/        # Build outputs
```

### API Structure
```
AI interface/trellix-wise/
├── main.py              # FastAPI application
├── prompts/             # Jinja2 templates
├── sample_data/         # Test data
└── tests/               # Test suite
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📄 License

This project is developed for Trellix endpoint product monitoring and analysis.

## 📞 Support

For support and questions regarding AI-Healthops, please refer to the documentation in each component directory:
- Desktop App: `AI-Healthops/README.md`
- AI API: `AI interface/README.md`

---

**Built with ❤️ for intelligent system monitoring and AI-powered analysis**
