#!/bin/bash

# Start the integrated GraphQL API with Vertex AI

echo "Starting Trellix WISE GraphQL API..."

# Load environment variables if .env file exists
if [ -f .env ]; then
    export $(grep -v '^#' .env | xargs)
fi

# Check required environment variables
if [ -z "$GOOGLE_CLOUD_PROJECT_ID" ]; then
    echo "Warning: GOOGLE_CLOUD_PROJECT_ID not set. Please set it in .env file."
fi

# Start integrated API
echo "Starting GraphQL API on port 8000..."
python main.py

echo "Services available at:"
echo "  GraphQL API: http://localhost:8000/graphql"
echo "  GraphQL Playground: http://localhost:8000/graphql"
echo "  Health check: http://localhost:8000/health"
