import cv2
import os
import numpy as np

# Ruta donde se encuentran las imágenes de los usuarios
dataPath = r'C:\Users\geova\Documents\PlatformIO\Projects\proyectoPasedelista\Datos'

listaPersonas = os.listdir(dataPath)
print('Personas: ', listaPersonas)

labels = []
faceData = []
label = 0

for nameDir in listaPersonas:
    personaPath = os.path.join(dataPath, nameDir)
    print("Leyendo imagenes")

    for fileName in os.listdir(personaPath):
        print('Rostro: ', nameDir + '/' + fileName)
        labels.append(label)
        faceData.append(cv2.imread(os.path.join(personaPath, fileName), 0))
        image = cv2.imread(os.path.join(personaPath, fileName), 0)
    
    label += 1

cv2.destroyAllWindows()

face_recognizer = cv2.face.LBPHFaceRecognizer_create()

print('Entrenando')
face_recognizer.train(faceData, np.array(labels))

face_recognizer.write('Modelo_Rostros_2024.xml')
print('Modelo entrenado')
