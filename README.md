Proyecto por etapas - Reloj Fichador

Etapa #1: 
- pulsador genera un registro (fichada)
- generacion de un horario falso para cada registro
- imprimir por UART los registros

Etapa #2:
- Agregado del modulo RTC (modelo DS1302) y configurarlo por unica vez
 
Etapa #3:
- Agregado de almacenamiento de memoria externa (tipo SD)
 
Etapa #4:
- Exportación del registro en algun formato procesable (.txt, .csv o equivalentes)
 
Etapa #5:
- Integrar lector de tarjetas con sus registros, en reemplazo del boton pulsador
 
Etapa #6:
- Integrar el lector de huella con sus registros
 
Etapa #7:
- Agregado de una pantalla (módulo LCD 16x2)
  - muestreo en el mismo de aceptacion / fallo en lectura de tarjeta y/o huella
  - muestreo de la hora actual cuando el sistema se encuentra en modo espera
