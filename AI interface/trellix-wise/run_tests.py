#!/usr/bin/env python3
"""
Simple test runner for Trellix WISE API tests.

This script runs all available tests without requiring pytest,
useful for environments where pytest has compatibility issues.
"""

import asyncio
import subprocess
import sys
import os

def run_test_file(test_file):
    """Run a single test file and return the result."""
    try:
        print(f"\n{'='*60}")
        print(f"Running {test_file}")
        print('='*60)
        
        result = subprocess.run([sys.executable, test_file], 
                              capture_output=False, 
                              text=True)
        
        if result.returncode == 0:
            print(f"✅ {test_file} - PASSED")
            return True
        else:
            print(f"❌ {test_file} - FAILED (exit code: {result.returncode})")
            return False
            
    except Exception as e:
        print(f"❌ {test_file} - ERROR: {e}")
        return False

def main():
    """Run all test files."""
    print("Trellix WISE API Test Runner")
    print("=" * 60)
    
    # Change to project root directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)
    
    # Test files to run
    test_files = [
        "tests/test_client.py",
        "tests/test_runtime_integration.py"
    ]
    
    results = []
    
    for test_file in test_files:
        if os.path.exists(test_file):
            results.append(run_test_file(test_file))
        else:
            print(f"⚠️  {test_file} - NOT FOUND")
            results.append(False)
    
    # Summary
    print(f"\n{'='*60}")
    print("Test Summary")
    print('='*60)
    
    passed = sum(results)
    total = len(results)
    
    print(f"Tests run: {total}")
    print(f"Passed: {passed}")
    print(f"Failed: {total - passed}")
    
    if passed == total:
        print("🎉 All tests passed!")
        return 0
    else:
        print("⚠️  Some tests failed!")
        return 1

if __name__ == "__main__":
    sys.exit(main())
