import cv2
import requests
import numpy as np
import os
from flask import Flask, Response

app = Flask(__name__)

personName = "Heriberto"
# Ruta donde se encuentran las imágenes de los usuarios
dataPath = r"C:\Users\geova\Documents\PlatformIO\Projects\proyectoPasedelista\Datos"
personPath = os.path.join(dataPath, personName)

if not os.path.exists(personPath):
    print("Creando carpeta:", personPath)
    os.makedirs(personPath)

# IP de la ESP32-CAM
url = 'http://192.168.100.26/capture'

# Inicializar el clasificador de rostros
faceClassif = cv2.CascadeClassifier(cv2.data.haarcascades + "haarcascade_frontalface_default.xml")
count = 0

def generate():
    global count
    while True:
        try:
            # Captura la imagen desde la URL
            img_resp = requests.get(url)
            
            # Verificar el código de estado HTTP
            if img_resp.status_code == 200:
                img_array = np.array(bytearray(img_resp.content), dtype=np.uint8)
                frame = cv2.imdecode(img_array, -1)

                if frame is None:
                    print("No se pudo decodificar el frame")
                    break

                # Procesamiento de la imagen y detección de rostros
                gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
                faces = faceClassif.detectMultiScale(gray, 1.3, 5)

                for (x, y, w, h) in faces:
                    cv2.rectangle(frame, (x, y), (x+w, y+h), (0, 255, 0), 2)
                    rostro = frame[y:y + h, x:x + w]
                    rostro = cv2.resize(rostro, (720, 720), interpolation=cv2.INTER_CUBIC)
                    # Guardar la imagen del rostro detectado
                    cv2.imwrite(os.path.join(personPath, f'rostro_{count}.jpg'), rostro)
                    count += 1

                # Codificar la imagen en formato JPEG
                ret, buffer = cv2.imencode('.jpg', frame)
                frame = buffer.tobytes()

                # Generar el flujo de datos en formato multipart/x-mixed-replace
                yield (b'--frame\r\n'
                       b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')
            else:
                print(f"Error HTTP {img_resp.status_code}: No se pudo obtener la imagen")

        except requests.exceptions.RequestException as e:
            print(f"Error de conexión: {e}")
            break

@app.route('/video_feed')
def video_feed():
    return Response(generate(),
                    mimetype='multipart/x-mixed-replace; boundary=frame')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
