# CUBO MÁGICO

## DESCRIPCIÓN Y FUNCIONAMIENTO

### Este proyecto consiste en el control de algunas funciones de la domótica casera mediante un mando a distancia especial, "Cubo Mágico". Xiaomi dispone de uno y es el que me impulsó a realizar un Cubo Mágico DIY.

### Este Cubo Mágico está diseñado en 3D con fusión 360 e impreso con una Ender 3. En su interior dispone de:

* ESP32 lolin32_lite (microcontrolador)
* MPU-6050 (unidad de medición inercial o IMU)
* Batería LIPO 3.7 1100 mAh
* 1 condensador electrolítico 470 microfaradios
* 1 condensador cerámico de 10 nano faradios
* 1 condensador electrolítico de 2200 microfaradios
* 2 resistencias de 100 kilo ohmios
* Pequeña placa electrónica casera para interconectar todos los componentes

### Cada cara del Cubo Mágico está identificada por un número del 1 al 6 y mediante ésta electrónica es capaz de identificar la cara superior enviando por mqtt el número asignado a dicha cara.

### El programa ejecutado en el ESP32 determina el número de la cara superior y la envía por mqtt al broker para que después mediante nodered y Home Assistant se le asigne una acción de la domótica de tú casa. Esta interacción entre el Cubo Mágico y la domótica de tú casa se realizará mediante Alexa y las notificaciones accionables. Cada vez que muevas el Cubo Mágico y se determine una nueva cara superior del mismo, Alexa te preguntará si quieres realizar una acción. Dependiendo de tu respuesta la acción se ejecutará o no.

### Tanto el ESP32 como el MPU-6050 se programan para que ahorren energía, yéndose el ESP32 a Deep Sleep si el cubo no se usa y desactivando el MPU-6050. Cada intervalo programado, el ESP32 se despertará y enviará el valor de carga de batería y si este es menor a un valor determinado avisará mediante Alexa para que lo pongas a cargar. El tiempo de arranque automático del Deep Sleep es configurable mediante la variable TIME_TO SLEEP.

### Para salir de forma manual de Deep Sleep y poder interactuar con el Cubo Mágico se utiliza la opción de despertar el ESP32 mediante un pin "touch". Para ello he conectado un cable al pin "touch" 4 GPIO 4 y se saca a la cara 1 mediante una sencilla lámina de cobre. Al tocar ésta lámina de cobre durante unos segúndos el Cubo Mágico sale del Deep Sleep detectando que está en la cara 1 comenzando la interactuación con Alexa.

### En mi caso he diseñado las siguientes acciones:

* Cara 1: Espera. Alexa te informa de que si no le das una orden en 30 segundos, el Cubo Mágico se va a dormir. Como orden se entiende el situar el Cubo Mágico en otra cara diferente a la 1
  
* Cara 2: Ambiente Salón. En ésta cara Alexa te pregunta si quieres ambientar el salón para ver la TV. Si respondes de forma afirmativa apaga la luz de la sala y enciende una lámpara pequeña

* Cara 3. Netflix. Alexa te pregunta si quieres ver Netflix y si respondes de forma afirmativa enciende la TV y te pone Netflix
  
* Cara 4 Prime. Alexa te pregunta si quieres ver Prime y si respondes de forma afirmativa enciende la TV y te pone Prime Video

* Cara 5. Temperaturas. Alexa te pregunta si quieres saber las temperaturas de la casa y si respondes de forma afirmativa Alexa te informa de las temperaturas de una serie de estancias

* Cara 6. Batería. En esta cara Alexa te pregunta si quieres obtener los valores actuales de batería del Cubo Mágico o cárgalo hasta el 100%. Si la respuesta es afirmativa te va informando de la carga de la batería
  
### Ante las ordenes de las caras 2, 3, 4 y 5 el cubo Mágico las ejecuta y pasados 120 segundos sin una nueva orden el Cubo Mágico pasa a Deep Sleep para ahorra batería. Cuando está en la cara 1, pasados 30 segundos sin ordenes el Cubo pasa a Deep Sleep. Solo cuando está en la cara 6 el Cubo Mágico no pasa a Deep Sleep porque entiende que estas cargando batería y cuando alcance el 100% lo situaras en la cara 1 u otra cara. El tiempo de 120 segundos es configurable mediante la variable t_reinit.

### Las variables de configuración del Cubo Mágico son accesibles en el archivo main.cpp y pueden modificarse para disponer de los valores que requiera cada usuario. Además, mediante Nodered y mqtt son accesibles para su reconfiguración las variables TIME_TO_SLEEP y t_init, sin tener que volver a cargar el código. También mediante Nodered y mqtt se puede reiniciar el ESP32, orden RESET, y se puede enviar a Deep Sleep, orden IR A DORMIR.

### El código es actualizable vía OTA pero el Cubo Mágico debe estar en la cara 6, ya que es la única cara que no va a Deep Sleep de forma automática.

### Toda la información necesaria para realizar este proyecto se comparte en este repositorio y la puedes encontrar organizada en las siguiente carpetas o archivos:

* Carpeta 3D. En ella está todo lo necesario para imprimir o modificar el diseño del Cubo Mágico
  
* Carpeta Esquemas. En ella está el conexionado de los dispositivos electrónicos del Cubo Mágico

* Carpeta Nodered. En ella está el programa de nodos que controlan las Alexa así como las acciones de la domótica. Algunos nodos deberán configurarse con los datos de tus dispositivos Alexa, Broker MQTT y Home Assistant
  
* Carpeta src. En ella está el archivo main.ccp con el código del programa del ESP32. Dentro de este archivo deberás rellenar una serie de variables para que el Cubo Mágico se conecte al WIFI y al Broker MQTT de tú casa
  
* Carpeta Foto-Video. En ella hay fotos y vídeos del funcionamiento del prototipo de mi Cubo Mágico

### El Cubo Mágico es adaptable a cualquier usuario aunque entiendo que las caras 1 y 6 se deben respetar para que realicen las mismas funciones. Las cars 2, 3 , 4 y 5 pueden modificarse para realizar acciones diferentes a las que yo hago en este proyecto. Para ello se debe modificar los nodos de Nodered que corresponden a esas caras.

## PASOS PARA INTERACTUAR CON EL CUBO MÁGICO

### Los pasos a realizar para interactuar con el Cubo Mágico son:

* La primera acción a realizar es elegir que Alexa será la que va a interactuar con el Cubo Mágico. En Nodered se debe pulsa sobre la Alexa elegida y ésta responderá aceptando la interacción

* Pulsa durante unos segundos la lámina metalizada de la cara 1 para que el Cubo Mágico salga del Deep Sleep

* El Cubo Mágico determinará que está en la cara 1 y te propondrá que le des ordenes, vamos que le sitúes en otra cara.

* Cuando le sitúes en otra cara, ésta será detectada y la Alexa te preguntará si quieres ejecutar la acción programada

* Si la respuesta es afirmativa se ejecutará y espera a una nueva orden o se va a Deep Sleep pasados 120 segundos

## PROBLEMAS DETECTADOS

### Durante la realización del proyecto he tenido problemas con el arranque del ESP32 debido a los picos de consumos, por eso el condensador de 2200 microfaradios en paralelo con la batería. Seguro que hay una forma más correcta de solucionarlo, pero ésta es la que a mí me funcionó. También es posible que sea debido a que mi batería LIPO de 1100 mAh está un poco "cascada", tengo otra en camino para probar.

### Al cargar el programa por el puerto serie, me da un error en dicho puerto. Para que cargue debo desconectar o la placa o el MPU-6050. No entiendo el porqué, ya que creo que no utilizo ninguno de los pines problemáticos del ESP32. Una vez cargado es necesario pulsar el RESET de la placa ESP32.

### En algunas ocasiones la actualización por OTA no detecta el nombre del host "Cubo_Magico" dando error de carga. PAra evitarlo tengo que introducir en el archivo platformio.ini la dirección IP directamente. No es un problema se comenta una de las opciones en el platformio.ini y la actualización por OTA funciona al 100% sin la necesidad de pulsar el RESET de la placa ESP32 tras actualizar el código.

### No fui capaz de utilizar el pin INT del MPU-6050 para despertar al ESP32 del Deep Sleep, por eso usé el despertar mediante pin "touch". Si alguien lo consigue que lo comparta.

### La Alexa que interactúa con el Cubo Mágico se selecciona mediante un nodo en nodered y asigna esa Alexa a la "alexa_media_last_called". Después se despierta el Cubo Mágico y comienza la interactuación. En ocasiones falla, no he conseguido solucionarlo. Como alternativa está sustituir en los nodos de nodered "alexa_media_last_called" por el nombre de la Alexa que interactuará y olvidarnos de seleccionar con anterioridad que Alexa va a interactuar.

## ENLACES Y AGRADECIMIENTOS

### Este proyecto lo tenía en mente desde hace un tiempo y tras el video del "Loco y su Tecnología" sobre las notificaciones accionables con Alexa me decidí a realizarlo y disfrutar de unir un ESP32, MQTT, Alexa, Home Assistant y un poco de electrónica para realizar mi "Cubo Mágico DIY".

* `https://randomnerdtutorials.com/esp32-touch-wake-up-deep-sleep/`
* `https://github.com/ccorderor/alexa-actions`

* Un Loco y su Tecnología, `https://www.youtube.com/channel/UC2zp7AWsYhZaGmYTjP8hZ7A`. Por su magnífico canal de Youtube donde comparte todos sus conocimientos
* `https://randomnerdtutorials.com`. Donde puedes obtener información muy didáctica de cómo empezar a programar cualquier microcontrolador

## Realizado por gurues (gurues@3DO ~ 2022 ~  ogurues@gmail.com)
