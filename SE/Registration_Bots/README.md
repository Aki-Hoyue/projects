# Code portion of Project
This folder contains my code reproduction of the paper "[Demystifying the underground ecosystem of account registration bots](https://doi.org/10.1145/3540250.3549090)" in which registration bots bypass three common human detection services(SMS, CAPTCHA, IP proxy).

## Author
    - Hoyue
    - Blog: https://hoyue.fun

## Construction

* `sms.py`: This code is used to send SMS to a random phone number and get the confirmation code from the SMS.
* `sms_opt.py`: This code is an optimized version of sms.py that uses the Caiyun v3 API(with CAPTCHA) to send SMS.
* `client.py`: This code does a simulates a POST request to the server.py endpoint with a random proxy IP from the proxy IP pool.
* `server.py`: This code does a simulates to receive a POST request from the client.py endpoint with a random proxy IP from the proxy IP pool.

## SMS workflow
[![image.png](https://i.postimg.cc/HsTyVpRK/image.png)](https://postimg.cc/SjtsPpy7)

1. Fetching Random Phone Number:

    Utilizes web scraping techniques to obtain a random phone number from a specific website (https://us-phone-number.com/US-Phone-Number/).

2. Sending SMS:

    Utilizes an SMS sending API (https://biz.caiyunapp.com/v1/send_sms_code) to simulate sending an SMS to the obtained random phone number.

3. Waiting for SMS and Retrieving Confirmation Code:

    Constructs a URL to check for SMS (https://us-phone-number.com/sms/{random_phone_number}).
    Checks the URL for the arrival of an SMS containing a verification code (confirmation_code) by scraping the website's HTML content.

4. Handling Timeout:

    Implements a timeout mechanism while waiting for the SMS containing the verification code. If the timeout period ends without receiving the SMS, it exits the process.

5. Displaying Output:

    Outputs information such as the randomly generated phone number, successful sending of SMS (if applicable), waiting for SMS indication, and displays the received verification code or a timeout message.

## IP proxy simulator workflow
[![image.png](https://i.postimg.cc/Y9wSZRwV/image.png)](https://postimg.cc/t12yVhJt)

1. Fetching Proxy IPs:

    Scrapes proxy IPs from specified target URLs (http://www.kxdaili.com/dailiip.html, http://www.kxdaili.com/dailiip/2/1.html) using web scraping techniques.
    Stores the fetched proxy IPs in a list.

2. Sending POST Request:

    Defines a function postTest to perform a POST request to the specified server endpoint (http://127.0.0.1:52525/post_endpoint) with provided sample data (username and password).
    Randomly selects a proxy IP from the pool of scraped proxy IPs.
    Constructs headers with an X-Forwarded-For field containing the selected proxy IP, simulating the use of a proxy server.
    Executes the POST request with the provided data, headers, and the selected proxy IP.

3. Handling Responses:

    Checks the response status code:
    If status code is 200 (OK), prints a success message along with the response content in JSON format.
    If the status code indicates failure, prints an error message with the corresponding status code.

4. Execution:

    Fetches the proxy IP pool using the getProxies() function and displays the obtained proxy IPs.
    Initiates the process of sending a POST request with a random proxy IP to the specified endpoint.

5. Output:

    Outputs information regarding the proxy IP pool, the selected proxy IP, the status of the POST request (success or failure), and the response content in case of success.

## CAPTCHA Reproduction
For the reproduction of CAPTCHA recognition, I used a set of "end-to-end" CAPTCHA recognition models trained on CNNs, trained by deep learning plus a large amount of data to simulate the results.
Model use reference: [https://github.com/dee1024/pytorch-captcha-recognition](https://github.com/dee1024/pytorch-captcha-recognition) and [https://github.com/czczup/Captcha-Recognition](https://github.com/czczup/Captcha-Recognition)

