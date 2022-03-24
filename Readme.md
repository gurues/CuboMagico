# CUBO MÁGICO

## DESCRIPCIÓN Y FUNCIONAMIENTO

### Este proyecto consiste en el control de algunas funciones de la domótica casera mediante un mando a distancia especial, "Cubo Mágico". Xiaomi dispone de uno y es el que me impuldo a realizar un Cubo Mágigo DIY'.'

### Este Cubo Mágico está diseñado en 3D con fusión 360 e impreso con una Ender. En su interior dipone de':'

* ESP32 lolin32_lite (microcontrolador)
* MPU-6050 (unidad de medición inercial o IMU)
* Batería LIPO 3.7 1100 mAh
* 1 condensador electrolítico 470 microfaradios
* 1 condensador ceramico de 10 nano faradios
* 1 condensador electrolítico de 2200 microfaradios
* 2 resistecias de 100 kilo ohmios
* Pequeña placa electronica casera para interconectar todos los componentes

### Cada cara del Cubo Mágico está identificada por un número del 1 al 6 y mediante esta electrónica es capaz de identificar la cara superior enviando por mqtt el número asignado a dicha cara'.'

### El programa ejecutado en el ESP32 determina el número de la cara superior y la envía por mqtt al broker para que después mediante nodered y Home Assistant se le asigne una acción de la domótica de tu casa. Esta ineracción entre el Cubo Mágico y la domótica de tú casa se realizará mediante Alexa. Cada vez que muevas el Cubo Mágico y se determine una nueva cara superior del mismo, Alexa te preguntará si quieres realizar una acción. Dependiendo de tu respuesta ésta se ejecutará o no"."

### Tanto el ESP32 como el MPU-6050 se programan para que ahorren energía, yendose el ESP32 a deep Sleep si el cubo no se usa y desactivando el MPU-6050. Cada intervalo programado el ESP32 se despertará y enviará el valora de carga de batería y si este es menor a un valor determinado avisará mediante Alexa para que lo pongas a cargar'.'

### Toda la información necesaría para realizar este proyecyo se comparte en este repositorio y la puedes encontrar en las siguientee carpetas o archivos':'

* Carpeta 3D. En ella está todo lo necesario para imprimir o modificar el diseño del Cubo Mágico
* Carpeta Esquemas. En ella está el conexionado de los disositivos electrónicos del Cubo Mágico
* Carpeta Nodered. En ella está el programa de nodos que controlan las Alexa así como las acciones de la domótica. Algunos nodos deberán configurarse con los datos de tus dispositivos Alexa, Broker MQTT y Home Assistant
* Carpeta src. En ella está el archivo main.ccp con el código del programa del ESP32. Dentro de este archivo deberas rellenar una serie de variables para que el Cubo Mágico se conecte al WIFI y al Broker MQTT de tu casa
