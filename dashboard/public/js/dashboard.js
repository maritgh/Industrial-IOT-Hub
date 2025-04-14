document.addEventListener('DOMContentLoaded', function() {
    // DOM Elements
    const loginForm = document.getElementById('login-form');
    const loginSection = document.getElementById('login-section');
    const loginError = document.getElementById('login-error');
    const locationSection = document.getElementById('location-section');
    const dashboardSection = document.getElementById('dashboard-section');
    const currentUserElement = document.getElementById('current-user');
    const dashboardUserElement = document.getElementById('dashboard-user');
    const logoutBtn = document.getElementById('logout-btn');
    const dashboardLogoutBtn = document.getElementById('dashboard-logout-btn');
    const backBtn = document.getElementById('back-to-locations');
    const currentLocationElement = document.getElementById('current-location');
    const updateTimeElement = document.getElementById('update-time');
    const locationCards = document.querySelectorAll('.location-card');
    const sidebarItems = document.querySelectorAll('.sidebar-menu li');
    const tabContents = document.querySelectorAll('.tab-content');

    // Valid users for authentication
    const validUsers = {
        'admin': 'henk',
        'username': 'henk', 
        'user2': 'henk'
    };

    // MQTT Client
    let client = null;
    let username = '';
    let dataTopic = 'stedin/data/';
    let currentLocation = 'rotterdam';
    
    // Charts
    let performanceChart, temperatureChart, humidityChart;
    
    // Initialize charts
    function initCharts() {
        // Performance Chart
        const performanceCtx = document.getElementById('performance-chart').getContext('2d');
        performanceChart = new Chart(performanceCtx, {
            type: 'line',
            data: {
                labels: Array.from({length: 24}, (_, i) => `${i}:00`),
                datasets: [{
                    label: 'Temperature (°C)',
                    data: generateRandomData(18, 26, 24),
                    borderColor: '#00a0df',
                    backgroundColor: 'rgba(0, 160, 223, 0.1)',
                    tension: 0.4,
                    fill: true
                }, {
                    label: 'Humidity (%)',
                    data: generateRandomData(40, 60, 24),
                    borderColor: '#7ac142',
                    backgroundColor: 'rgba(122, 193, 66, 0.1)',
                    tension: 0.4,
                    fill: true
                }, {
                    label: 'Power Output (MW)',
                    data: generateRandomData(10, 15, 24),
                    borderColor: '#004494',
                    backgroundColor: 'rgba(0, 68, 148, 0.1)',
                    tension: 0.4,
                    fill: true
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: true,
                plugins: {
                    legend: {
                        position: 'top',
                    }
                },
                scales: {
                    y: {
                        beginAtZero: false,
                        grid: {
                            color: 'rgba(0, 0, 0, 0.05)'
                        }
                    },
                    x: {
                        grid: {
                            color: 'rgba(0, 0, 0, 0.05)'
                        }
                    }
                }
            }
        });
        
        // Temperature Chart
        const temperatureCtx = document.getElementById('temperature-chart').getContext('2d');
        temperatureChart = new Chart(temperatureCtx, {
            type: 'line',
            data: {
                labels: Array.from({length: 24}, (_, i) => `${i}:00`),
                datasets: [{
                    label: 'Temperature (°C)',
                    data: generateRandomData(18, 26, 24),
                    borderColor: '#00a0df',
                    backgroundColor: 'rgba(0, 160, 223, 0.1)',
                    tension: 0.4,
                    fill: true
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: true,
                plugins: {
                    legend: {
                        position: 'top',
                    }
                },
                scales: {
                    y: {
                        beginAtZero: false,
                        grid: {
                            color: 'rgba(0, 0, 0, 0.05)'
                        }
                    },
                    x: {
                        grid: {
                            color: 'rgba(0, 0, 0, 0.05)'
                        }
                    }
                }
            }
        });
        
        // Humidity Chart
        const humidityCtx = document.getElementById('humidity-chart').getContext('2d');
        humidityChart = new Chart(humidityCtx, {
            type: 'line',
            data: {
                labels: Array.from({length: 24}, (_, i) => `${i}:00`),
                datasets: [{
                    label: 'Humidity (%)',
                    data: generateRandomData(40, 60, 24),
                    borderColor: '#7ac142',
                    backgroundColor: 'rgba(122, 193, 66, 0.1)',
                    tension: 0.4,
                    fill: true
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: true,
                plugins: {
                    legend: {
                        position: 'top',
                    }
                },
                scales: {
                    y: {
                        beginAtZero: false,
                        grid: {
                            color: 'rgba(0, 0, 0, 0.05)'
                        }
                    },
                    x: {
                        grid: {
                            color: 'rgba(0, 0, 0, 0.05)'
                        }
                    }
                }
            }
        });
    }
    
    // Generate random data for charts
    function generateRandomData(min, max, count) {
        return Array.from({length: count}, () => Math.random() * (max - min) + min);
    }
    
    // Login Form Handler
    loginForm.addEventListener('submit', function(e) {
        e.preventDefault();
        
        username = document.getElementById('username').value;
        const password = document.getElementById('password').value;
        
        if (validUsers[username] && validUsers[username] === password) {
            loginError.style.display = 'none';
            connectToBroker(username, password);
        } else {
            loginError.textContent = 'Invalid username or password';
            loginError.style.display = 'block';
        }
    });
    
    // Connect to MQTT Broker
    function connectToBroker(username, password) {
        const host = window.location.hostname;
        const port = 1884; // MQTT over WebSockets port
        const clientId = 'stedin_dashboard_' + Math.random().toString(16).substr(2, 8);

        const wsProtocol = 'ws';
        const url = `${wsProtocol}://${host}:${port}`;

        client = mqtt.connect(url, {
            clientId: clientId,
            clean: true,
            username: username,
            password: password
        });

        client.on('connect', function() {
            console.log('Connected to MQTT broker');
            
            // Subscribe to data topics
            client.subscribe(dataTopic + '#', function(err) {
                if (!err) {
                    // Show location selection screen
                    loginSection.style.display = 'none';
                    locationSection.style.display = 'block';
                    currentUserElement.textContent = username;
                    
                    console.log('Subscribed to data topics');
                } else {
                    console.error('Failed to subscribe:', err);
                    loginError.textContent = 'Failed to connect to data feed: ' + err.message;
                    loginError.style.display = 'block';
                }
            });
        });

        client.on('error', function(err) {
            console.error('MQTT Error:', err);
            loginError.textContent = 'Connection error: ' + err.message;
            loginError.style.display = 'block';
        });

        client.on('message', function(topic, message) {
            try {
                const data = JSON.parse(message.toString());
                processIncomingData(topic, data);
            } catch (e) {
                console.error('Failed to parse message:', e);
            }
        });
    }
    
    // Process incoming MQTT data
    function processIncomingData(topic, data) {
        // Only process data for the current location
        if (topic.includes(currentLocation)) {
            console.log('Received data:', data);
            
            // Update UI with new data
            if (data.temperature) {
                document.getElementById('temp-value').textContent = data.temperature.toFixed(1);
                document.getElementById('detail-temp-value').textContent = data.temperature.toFixed(1);
            }
            
            if (data.humidity) {
                document.getElementById('humidity-value').textContent = data.humidity.toFixed(0);
                document.getElementById('detail-humidity-value').textContent = data.humidity.toFixed(0);
            }
            
            if (data.power) {
                document.getElementById('power-value').textContent = data.power.toFixed(1);
            }
            
            if (data.status) {
                const statusElement = document.getElementById('status-value');
                statusElement.textContent = data.status;
                
                // Update status class
                statusElement.className = '';
                if (data.status === 'Normal') {
                    statusElement.classList.add('status-normal');
                } else if (data.status === 'Warning') {
                    statusElement.classList.add('status-warning');
                } else if (data.status === 'Critical') {
                    statusElement.classList.add('status-danger');
                }
            }
            
            // Update timestamp
            updateTimeElement.textContent = new Date().toLocaleTimeString();
        }
    }
    
    // Simulate incoming data for demo purposes
    function simulateDataStream() {
        setInterval(() => {
            const data = {
                temperature: Math.random() * (26 - 18) + 18,
                humidity: Math.random() * (60 - 40) + 40,
                power: Math.random() * (15 - 10) + 10,
                status: Math.random() > 0.8 ? 'Warning' : 'Normal'
            };
            
            processIncomingData(`stedin/data/${currentLocation}`, data);
        }, 5000); // Update every 5 seconds
    }
    
    // Handle location selection
    locationCards.forEach(card => {
        card.addEventListener('click', () => {
            currentLocation = card.dataset.location;
            const locationName = card.querySelector('h3').textContent;
            
            // Update UI
            currentLocationElement.textContent = locationName;
            
            // Change to dashboard view
            locationSection.style.display = 'none';
            dashboardSection.style.display = 'block';
            dashboardUserElement.textContent = username;
            
            // Initialize charts
            initCharts();
            
            // Start simulating data updates
            simulateDataStream();
        });
    });
    
    // Handle sidebar navigation
    sidebarItems.forEach(item => {
        item.addEventListener('click', () => {
            // Remove active class from all sidebar items
            sidebarItems.forEach(i => i.classList.remove('active'));
            
            // Add active class to clicked item
            item.classList.add('active');
            
            // Hide all tab contents
            tabContents.forEach(tab => tab.classList.remove('active'));
            
            // Show selected tab content
            const tabId = item.dataset.tab + '-tab';
            document.getElementById(tabId).classList.add('active');
        });
    });
    
    // Back button functionality
    backBtn.addEventListener('click', (e) => {
        e.preventDefault();
        dashboardSection.style.display = 'none';
        locationSection.style.display = 'block';
    });
    
    // Logout functionality
    function logout() {
        if (client) {
            client.end();
            client = null;
        }
        
        // Reset UI
        loginSection.style.display = 'block';
        locationSection.style.display = 'none';
        dashboardSection.style.display = 'none';
        
        // Clear forms
        document.getElementById('username').value = '';
        document.getElementById('password').value = '';
        loginError.style.display = 'none';
    }
    
    logoutBtn.addEventListener('click', logout);
    dashboardLogoutBtn.addEventListener('click', logout);
});