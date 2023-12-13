"""
client.py
    This code does a simulates a POST request to the server.py endpoint with a random proxy IP from the proxy IP pool.

Dependencies:
    - requests
    - lxml
    - random

Usage:
    - python client.py
    
Author:
    - Hoyue

Expected output example (if successful):
    - Proxy IP Pool:
    - ...
    - Sending POST request with a random proxy IP...
    - Selected Proxy IP:
    - ...
    - POST Request Successful!
    - Response:
    - ...
"""


import requests
from lxml import html
import random

def getProxies():
    # Target URLs to scrape proxy IPs from
    target_urls = [
        "http://www.kxdaili.com/dailiip.html",
        "http://www.kxdaili.com/dailiip/2/1.html"
    ]
    proxy_ip_pool = []  # Initialize an empty list to store the proxy IPs
    
    # Scrape the proxy IPs from the target URLs
    for url in target_urls:
        response = requests.get(url)
        if response.status_code == 200:
            # Parse the HTML response
            tree = html.fromstring(response.content)
            rows = tree.xpath("//table[@class='active']//tr")[1:]
            for tr in rows:
                ip = "".join(tr.xpath('./td[1]/text()')).strip()
                port = "".join(tr.xpath('./td[2]/text()')).strip()
                proxy_ip = f"{ip}:{port}"
                proxy_ip_pool.append(proxy_ip)  # Append each proxy IP to the list
                yield proxy_ip  # Yield the proxy IP as well
    return proxy_ip_pool  # Return the complete proxy IP pool list

def postTest(url, username, password, proxies):
    # Sample data to be sent
    data = {
        "username": username,
        "password": password
    }

    # Select a random proxy IP
    selected_proxy = random.choice(proxies)
    print(f"Selected Proxy IP: {selected_proxy}")

    # Create headers with X-Forwarded-For containing the selected proxy IP
    headers = {
        "X-Forwarded-For": selected_proxy
    }

    try:
        response = requests.post(url, json=data, headers=headers)
        
        if response.status_code == 200:
            print("POST Request Successful!")
            print("Response:")
            print(response.json())
        else:
            print(f"POST Request Failed with status code: {response.status_code}")

    except requests.RequestException as e:
        print(f"Error in making the POST request: {e}")

if __name__ == "__main__":
    # Get the proxy IP pool
    proxies = list(getProxies())
    print("Proxy IP Pool:")
    for proxy in proxies:
        print(proxy)
    
    print("Sending POST request with a random proxy IP...")
    
    # URL to send the POST request to
    url = "http://127.0.0.1:52525/post_endpoint"
    username = "test"
    password = "test"
    postTest(url, username, password, proxies)
