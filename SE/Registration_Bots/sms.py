"""
sms.py
    This code is used to send SMS to a random phone number and get the confirmation code from the SMS.

Dependencies:
    - requests
    - bs4
    - random
    - re
    - time

Usage:
    - python sms.py
    
Author:
    - Hoyue
    
Expected output example (if successful):
    - SMS Number: 15103899190
    - Successful sending of SMS to 15103899190
    - Waiting for SMS...
    - 123456
"""


import requests
from bs4 import BeautifulSoup
import random
import re
import time


def get_phone_number():
    url = "https://us-phone-number.com/US-Phone-Number/"
    response = requests.get(url)
    
    # Parse the HTML response and get numbers in h4 tags
    soup = BeautifulSoup(response.text, "html.parser")
    phone_numbers = soup.find_all("h4", class_="number-boxes-item-number")
    # Get a random phone number
    phone_number = random.choice(phone_numbers).text.strip()
    # Remove "+" and spaces
    phone_number = phone_number.replace("+", "").replace(" ", "")
    
    return phone_number


def get_sms(sms_url):
    response = requests.get(sms_url)
    soup = BeautifulSoup(response.text, "html.parser")
    sms_divs = soup.find_all("div", class_="col-xs-12 col-md-8")
    
    for index in sms_divs:
        get_content = index.text.strip()
        # Get the confirmation code
        match = re.search(r"\[caiyun\]Your verification code is (\d{6}), it is valid in five minutes.", get_content)
        if match:
            confirmation_code = match.group(1)
            return confirmation_code

    return None

def send_sms(sms_number):
    # SMS sending API by Caiyun v1(without CAPTCHA)
    url = "https://biz.caiyunapp.com/v1/send_sms_code"
    
    # Data to be sent
    data = {
        "phone_num": sms_number[1:],    # Phone number portion
        "area_code": sms_number[0]    # Country code portion
    }
    
    # Headers are required to send the request
    headers = {
        "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36",
        "Referer": "https://fanyi.caiyunapp.com/",
        "Cy-Token": "token 9876032166"     # Token obtained from packet capture
    }

    try:
        response = requests.post(url, data=data, headers=headers)

        if response.status_code == 200:
            print(f"Successful sending of SMS to {sms_number}")
            return True
        else:
            print(f"Error with: {response.status_code}")
            return False
    except Exception as e:
        print(f"Error: {str(e)}")


def main():
    # Get a random phone number
    sms_number = get_phone_number()
    
    # Construct the URL to get SMS
    sms_url = f"https://us-phone-number.com/sms/{sms_number}"
    print("SMS Number: " + sms_number)
    
    send_sms(sms_number)
    
    timeout = 10
    while 1:
        timeout -= 1
        status = get_sms(sms_url)
        if status:
            print(get_sms(sms_url))
            break
        elif not status and timeout != 0:
            time.sleep(1)
            print("Waiting for SMS...")
        elif timeout == 0:
            print("Timeout")
            break
        
    print(status)

if __name__ == "__main__":
    main()
