Architecture Overview

Home Assistant's (HA) "MQTT Discovery" is an automation feature that automatically finds, configures, and adds compatible smart devices and services to your Home Assistant dashboard. 
Instead of manually typing in IP addresses or editing raw yaml files, the HA Discovery subsystem handles setup for you which greatly simplifies the setup experience.

Your AlphaMon device aids HA's Discovery process by publishing specially formatted MQTT data packets to predefined HA addresses.  These data packets tell HA where to find the your AlphaMon's data and how it is formatted.
By default, HA expects the Discovery packets to be published to the MQTT Broker's root directory at #homeassistant. However, this architecture doesn't readily support multiple user groups sharing a single broker service so some additional configuration steps are typically required.
The following sections describe how to install HA on a Linux server running the Aphache2 web server and configure it to expose its Dashboard via private or public web pages. 
We assume the web browser will be running on a Windows deskop PC for the sake of this example, but you can run the broswer on any suitable device such as a tablet or smart phone. 

The following AlphaMon/HA system setup assumes a multi-tier network topology to separate your LAN from IoT telemetry. 
Note: When deploying HA on an existing Linux host that also hosts the primary MQTT broker, a network isolation layer is required and this is discussed later in this tutorial.
For this discussion we'll be assuming the following network architecture, which includes the creation of a KVM hypervisor on the Linux server to host the HA application:

[ LAN Subnet: 10.1.1.0/24 ]          [ Host OS: Ubuntu (10.1.1.84) ]      [ KVM Private Network ]
┌─────────────────────────┐          ┌─────────────────────────────┐      ┌─────────────────────┐
│  Desktop PC (10.1.1.45) ├─[HTTP/80]┼─► Apache2 Reverse Proxy     ├──────┼─► HA OS VM          │
└─────────────────────────┘          │   └─ listens: 192.168.122.1 │      │   ├─ 192.168.122.166│
                                     │   └─ Mosquitto (Port 1883)  │◄─TCP─┤   └─ HA Port: 8123  │
                                     └─────────────────────────────┘      └─────────────────────┘

Part A: ** KVM Environment Provisioning **

1. Verify CPU extension support and hypervisor availability on the host OS:

    $ egrep -c '(vmx|svm)' /proc/cpuinfo
    $ sudo apt update && sudo apt install -y cpu-checker
    $ kvm-ok

2. Dependencies & Image Staging
Install the virtualization management stack and pull down the raw UEFI appliance build:

    $ bashsudo apt install -y qemu-kvm libvirt-daemon-system libvirt-clients bridge-utils virtinst
    $ sudo mkdir -p /var/lib/libvirt/images/haos && cd /var/lib/libvirt/images/haos
    $ sudo wget -L https://github.com
    $ sudo unxz haos_ova-14.2.qcow2.xz
    
3. VM Deployment Configuration
Provision the domain instance targeting the virtual NAT switch interface (virbr0):

    $ sudo virt-install \
      --name homeassistant \
      --description "Home Assistant OS Appliance" \
      --os-variant generic \
      --ram 2048 \
      --vcpus 2 \
      --disk /var/lib/libvirt/images/haos/haos_ova-14.2.qcow2,bus=sata \
      --graphics none \
      --boot uefi \
      --network bridge=virbr0,model=virtio \
      --import \
      --noautoconsole

    $ sudo virsh autostart homeassistant
    
To monitor allocation states and track IP binding leases within the internal gateway partition:

    $ arp -e -i virbr0
    # Target VM IP expected: 192.168.122.166

    
Part B: ** Apache2 Reverse Proxy & WebSocket Tunneling **

Because the VM sits within a nested NAT framework, Apache2 must handle reverse frame routing and persistent upstream WebSocket states.

1. Ensure the proxy runtime submodules are injected:

    $ sudo a2enmod proxy proxy_http proxy_wstunnel rewrite
    
2. Create Apache2 VirtualHost Configuration File:    

    <VirtualHost *:80>
        ServerName ha
        ServerAlias ha.local

        ProxyPreserveHost On
        ProxyRequests Off
        ProxyWebsocketFallbackToProxyHttp On

        # WebSocket Handshake Routing (Crucial for frontend state changes)
        RewriteEngine On
        RewriteCond %{HTTP:Upgrade} =websocket [NC]
        RewriteCond %{HTTP:Connection} upgrade [NC]
        RewriteRule /(.*)           ws://192.168.122.166:8123/$1 [P,L]

        # Standard HTTP Core Proxy Mapping
        # NOTE: Trailing slashes are syntactically required to prevent redirection mismatches
        ProxyPass / http://192.168.122.166:8123/ keepalive=On timeout=600
        ProxyPassReverse / http://192.168.122.166:8123/
    </VirtualHost>

3. Activate config definition layout and reload worker pools:

    $ sudo a2ensite homeassistant.conf
    $ sudo systemctl restart apache2


Part C: ** Client Subnet Route Injection **

To access the setup using a clean URL structure from a workstation on the 10.1.1.0/24 subnet, register the hostname mapping directly on the workstation.

1. Windows Desktop Host Routing

Append the following string to your Windoes hosts file, as Administrator (typically at C:\Windows\System32\drivers\etc\hosts). 

    10.1.1.84    ha.local ha
    
2. Home Assistant Proxy Whitelisting

By default, Home Assistant drops proxy connections outside its direct interface layer. Thus, you must declare the KVM virtual bridge gateway loop address inside the VM's file system layout.

To access the KVM hypervisor interface terminal session directly, run the virtual shell (virsh) utility:    

    $ sudo virsh console homeassistant
    
The KVM terminal session will prompt you to authenticate. Enter the username of root, (no password required).

Then, at the subsequent "#" system shell prompt, run the following commands to create a mountpoint directory and configure the HA configuration.yaml file inside that mountpoint:
    # mkdir -p /mnt/data/supervisor/homeassistant
    # cat << 'EOF' > /mnt/data/supervisor/homeassistant/configuration.yaml
    # default_config:

      http:
        use_x_forwarded_for: true
        trusted_proxies:
          - 192.168.122.1
      EOF

Terminate the KVM terminal session using the Ctrl+] key strokes then execute KVM domain reboot sequence:

    $ sudo virsh reboot homeassistant

  
Part D: ** 4. MQTT Broker Integration & Custom Discovery Prefix Mapping **

1. Next, we need to tell HA to load the MQTT Service which we'll use to connect to the MQTT Broker used by the AlphaMon to publish its energy data.

Navigate the HA web interface to: Settings -> Devices & Services -> Add Integration -> MQTT.

2. MQTT Broker Configuration

Assuming you're connecting to the default AlphaMon Platform public MQTT sandbox for testing, configure the HA MQTT Integration service as follows:

  Broker Host IP: mqtt.solargy.com.au
  Port: 1883
  Username: mqtt_test
  Password: mqtt_11111975

3. HA Discovery Prefix Schema Alignment

The Solargy MQTT service at https://mqtt.solargy.com.au hosts both public and private workspaces.
By default, The AlphaMon Platform is configured to point to the public "sandbox" workspace located at #AlphaMon:Demo on the Solargy MQTT server.  
You can use this sandbox to test your AlphaMon and MQTT configuration but be aware it is regularly purged so as to present a clean testing environment for all public users.

You can track your individual AlphaMon's data in the sandbox by its unique MAC Address.  The MAC Address appears in the MQTT broker as the letters "AM" followed by the last 6 charachers of your AlphaMon's physical MAC address.  For example "AM637690".

Since the default HA installation expects to find the MQTT Discovery topics in the root #homeassistant topic, you'll need to make the following changes to the standard HA configuration in order to use the sandbox.

Under the Home Assistant MQTT Integration card, click Configure -> Re-configure MQTT. Modify the Discovery input field to match the Solargy snadbox, as follows:

    AlphaMon:Demo/homeassistant

Save the change and click on the MQTT Service sub-menu (three vertical dots) and click on "Reload" to restart the MQTT integration service.

4. Valitate the Connection

To validate that HA can see your AlphaMon's MQTT data, follow these steps:

    - Navigate to the HA Settings menu option on the LHS menu
    - Click on "Entities" at the top of the screen
    - In the Search Bar at the top of the Entities page enter the name of one of your installed AlphaMon Data Definition Files (e.g. DDS6619)
    - If that DDF is enabled in your AlphaMon and its data is streaming into the MQTT Broker you'll see the packets displayed under the search bar.
    - You can now proceed to configure HA's dashboard using the data from that data source.
    
    

