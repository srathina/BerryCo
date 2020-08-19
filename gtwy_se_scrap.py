import requests
import pprint

response = requests.get('https://www.thethingsnetwork.org/gateway-data/country/se')
jsonstat = response.json()
p = pprint.PrettyPrinter(indent=2)
p.pprint(jsonstat['eui-b827ebffff1f5db7'])