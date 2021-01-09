import optparse
import requests
import sys
def parser(unparsed_args):
  parser = optparse.OptionParser(
    usage = "%prog [options]",
    description = "Transmit Atmega hex files over the air to the esp8266 module and initiate code update"
  )
  # destination ip and port
  group = optparse.OptionGroup(parser, "Destination")
  group.add_option("-i", "--ip",
    dest = "esp_ip",
    action = "store",
    help = "ESP8266 IP Address.",
    default = False
  )
  
  parser.add_option_group(group)
  # image
  group = optparse.OptionGroup(parser, "Image")
  group.add_option("-f", "--file",
    dest = "imageFile_Name",
    action= "store",
    help = "Image File Name",
    default = ""
  )
  parser.add_option_group(group)
  (options, args) = parser.parse_args(unparsed_args)
  return options

def uploadCode(host,filename):
  try:
    if(host!="0.0.0.0" and filename!=""):
      uploadLink="http://"+host+"/atmega"
      print("Uploading file...",end="")
      r=None
      with open(filename, 'rb') as f:
        r = requests.post(uploadLink, files={filename: f})
      print(r.text)
      if(r.status_code==200 and r.text=="1"):
        print("Uploaded")
        return 1
      else:
        print(r.text)
        print("Not uploaded")
  except Exception as e:
    print(e)
  return 0

def main(args):
  options=parser(args)
  tries=1
  while(tries>0):
    x=uploadCode(options.esp_ip,options.imageFile_Name)
    if(x==1):
      return 1
    if(tries==1):
      return x
    tries=tries-1

if __name__=="__main__":
  main(sys.argv)
  sys.exit(1)