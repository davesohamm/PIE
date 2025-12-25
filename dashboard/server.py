#!/usr/bin/env python3
"""
PIE Dashboard Server
Web interface for Performance Intelligence Engine
"""

from flask import Flask, render_template, jsonify, request
from flask_cors import CORS
import json
import os
import subprocess
import psutil
from datetime import datetime

app = Flask(__name__)
CORS(app)

# Store results
results_cache = []

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/system_info')
def system_info():
    """Get system information"""
    cpu_info = {
        'model': subprocess.getoutput("grep 'model name' /proc/cpuinfo | head -1 | cut -d':' -f2").strip(),
        'cores': psutil.cpu_count(logical=False),
        'threads': psutil.cpu_count(logical=True),
        'frequency': psutil.cpu_freq().current if psutil.cpu_freq() else 0,
        'usage': psutil.cpu_percent(interval=0.1)
    }
    
    memory = psutil.virtual_memory()
    mem_info = {
        'total': memory.total,
        'available': memory.available,
        'used': memory.used,
        'percent': memory.percent
    }
    
    return jsonify({
        'cpu': cpu_info,
        'memory': mem_info,
        'timestamp': datetime.now().isoformat()
    })

@app.route('/api/workloads')
def get_workloads():
    """Get available workloads"""
    workloads = [
        {
            'name': 'matmul',
            'description': 'Matrix Multiplication (Cache Locality Test)',
            'default_size': 1024,
            'category': 'memory'
        },
        {
            'name': 'vector_add',
            'description': 'Vector Addition (SIMD Throughput Test)',
            'default_size': 10000000,
            'category': 'compute'
        },
        {
            'name': 'memory_stream',
            'description': 'Memory Streaming (Bandwidth Test)',
            'default_size': 100000000,
            'category': 'memory'
        },
        {
            'name': 'branch_heavy',
            'description': 'Branch-Heavy Loop (Control Flow Test)',
            'default_size': 10000000,
            'category': 'control_flow'
        },
        {
            'name': 'reduction',
            'description': 'Parallel Reduction (Dependency Chain Test)',
            'default_size': 100000000,
            'category': 'compute'
        }
    ]
    
    return jsonify(workloads)

@app.route('/api/profile', methods=['POST'])
def profile_workload():
    """Profile a workload"""
    data = request.json
    workload = data.get('workload', 'matmul')
    size = data.get('size', 1024)
    
    try:
        # Run PIE profiler
        cmd = ['../build/pie', 'analyze', workload, '--size', str(size)]
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
        
        # Parse output (simplified)
        output = result.stdout + result.stderr
        
        # Extract metrics using simple parsing
        metrics = {
            'workload': workload,
            'size': size,
            'timestamp': datetime.now().isoformat(),
            'raw_output': output,
            'success': result.returncode == 0
        }
        
        results_cache.append(metrics)
        
        return jsonify(metrics)
    
    except subprocess.TimeoutExpired:
        return jsonify({'error': 'Profiling timed out'}), 500
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/api/optimize', methods=['POST'])
def optimize_workload():
    """Run optimization pipeline"""
    data = request.json
    workload = data.get('workload', 'matmul')
    size = data.get('size', 1024)
    
    try:
        cmd = ['../build/pie', 'optimize', workload, '--size', str(size)]
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=60)
        
        output = result.stdout + result.stderr
        
        optimization_result = {
            'workload': workload,
            'size': size,
            'timestamp': datetime.now().isoformat(),
            'raw_output': output,
            'success': result.returncode == 0
        }
        
        results_cache.append(optimization_result)
        
        return jsonify(optimization_result)
    
    except subprocess.TimeoutExpired:
        return jsonify({'error': 'Optimization timed out'}), 500
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/api/compare', methods=['POST'])
def compare_cpu_gpu():
    """Compare CPU vs GPU"""
    data = request.json
    workload = data.get('workload', 'matmul')
    size = data.get('size', 512)
    
    try:
        cmd = ['../build/pie', 'compare', workload, '--size', str(size)]
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=60)
        
        output = result.stdout + result.stderr
        
        comparison = {
            'workload': workload,
            'size': size,
            'timestamp': datetime.now().isoformat(),
            'raw_output': output,
            'success': result.returncode == 0
        }
        
        return jsonify(comparison)
    
    except subprocess.TimeoutExpired:
        return jsonify({'error': 'Comparison timed out'}), 500
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/api/results')
def get_results():
    """Get all cached results"""
    return jsonify(results_cache)

@app.route('/api/results/clear', methods=['POST'])
def clear_results():
    """Clear results cache"""
    global results_cache
    results_cache = []
    return jsonify({'success': True})

if __name__ == '__main__':
    print("🚀 PIE Dashboard Server Starting...")
    print("📊 Open http://localhost:5000 in your browser")
    app.run(host='0.0.0.0', port=5000, debug=True)

