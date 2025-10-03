from flask import Flask, request
import cv2
import numpy as np
import pytesseract
import datetime

app = Flask(__name__)

@app.route('/upload', methods=['POST'])
def upload_image():
    # Receive image data
    img_data = request.data
    np_arr = np.frombuffer(img_data, np.uint8)
    img = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)

    # Save image with timestamp
    filename = datetime.datetime.now().strftime("%Y%m%d_%H%M%S") + ".jpg"
    cv2.imwrite(filename, img)
    print(f"Image saved: {filename}")

    # Process image (gray + OCR)
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    text = pytesseract.image_to_string(gray, config='--psm 8')
    print("Detected Number Plate Text:", text.strip())

    return "Image received & processed", 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
