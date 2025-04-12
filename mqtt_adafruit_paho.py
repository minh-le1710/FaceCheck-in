import paho.mqtt.publish as publish
publish.single(
  "nhatminh1710/feeds/chamcong123",
  "1", # "0" if failure
  hostname="io.adafruit.com", port=8883,
  auth={'username':'nhatminh1710','password':'aio_gKzi73E8RHcNP0sxZHA5ITnsRn62'}
)
