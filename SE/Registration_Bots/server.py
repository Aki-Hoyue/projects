"""
server.py
    This code does a simulates to receive a POST request from the client.py endpoint with a random proxy IP from the proxy IP pool.

Dependencies:
    - flask

Usage:
    - python server.py
    Run the client.py script in another terminal window.

Author:
    - Hoyue
"""


from flask import Flask, request, jsonify

app = Flask(__name__)

# Endpoint for GET requests
@app.route('/post_endpoint', methods=['POST'])
def post_request_handler():
    if request.method == 'POST':
        data = request.json  # Get the JSON data from the request
        username = data.get('username')
        password = data.get('password')

        # Get client's IP or proxy IP
        client_ip = request.headers.get('X-Forwarded-For') or request.remote_addr  # Get client's IP or proxy IP

        # Get client's User-Agent to check if it's a bot
        user_agent = request.headers.get('User-Agent')
        if not user_agent:
            return jsonify({"error": "User-Agent is empty"}), 403  # Return 403 Forbidden if User-Agent is empty

        # Construct response data
        response_data = {
            "status": "ok",
            "username": username,
            "password": password,
            "ip": client_ip
        }

        return jsonify(response_data), 200

if __name__ == '__main__':
    # Run the Flask app on localhost, port 52525
    app.run(host='127.0.0.1', port=52525)

