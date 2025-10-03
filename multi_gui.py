import _thread
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.widgets import TextBox, Button
import numpy as np
import socket


HOST = "192.168.1.1"
# HOST = "127.0.0.1"
PORT = 288
soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
soc.connect((HOST, PORT))
soc.setblocking(True)
moving = False

# sample: "099:128\n". degree:cm
# nope: <1 st byte angle> <2nd byte c
CYBOT_MESSAGE_SIZE = 9

thetaCliff = []
valueCliff = []
thetaEdge = []
valueEdge = []
thetaObst = []
valueObst = []



class BotTracker:
    cybot_from_origin_x = 0.5
    cybot_from_origin_y = 0.5
    cybot_angle = np.pi / 2.
    cybot_map_from_org_x = 0.5
    cybot_map_from_org_y = 0.5
    cybot_map_angle = np.pi / 2.

    def reset(self):
        self.cybot_from_origin_x = 0.5
        self.cybot_from_origin_y = 0.5
        self.cybot_angle = np.pi / 2
        thetaObst.clear()
        valueObst.clear()
        thetaCliff.clear()
        valueCliff.clear()
        thetaEdge.clear()
        valueEdge.clear()
    
    def moved(self, distance):
        self.cybot_from_origin_x += distance * np.cos(self.cybot_angle)
        self.cybot_from_origin_y += distance * np.sin(self.cybot_angle)
        self.cybot_map_from_org_x += distance * np.cos(self.cybot_angle)
        self.cybot_map_from_org_y += distance * np.sin(self.cybot_map_angle)

    def turned(self, angle):
        rad_angle = angle * (np.pi / 180)
        self.cybot_angle += rad_angle
        self.cybot_map_angle += rad_angle


botTracker = BotTracker()
fig, ax = plt.subplots(subplot_kw={'projection': 'polar'}, figsize=(5,5))
# ax_map = plt.subplot(4, 1, 1)
# fig.tight_layout()
# ax_map.scatter([1], [1])
fig_map, ax_map = plt.subplots()

plt.figure(fig)

class CyBotOps:
    def begin(self, event):
        soc.send(b'm')
        botTracker.reset() # not reseting
        thetaCliff.clear()
        valueCliff.clear()
    def end(self, event):
        soc.send(b'e')
    def toggle(self, event):
        soc.send(b't')
    def quit(self, event):
        soc.send(b'q')
    def key_press(self, event):
        if event.key == 'w' or event.key == 'a' or event.key == 's' or event.key == 'd':
            global moving
            if moving == False:
                moving = True
                soc.send(event.key.encode('ascii'))
        elif event.key == 'm':
            soc.send(event.key.encode('ascii'))

    def key_unpress(self, event):
        if event.key == 'w' or event.key == 'a' or event.key == 's' or event.key == 'd':
            global moving
            moving = False
            soc.send('0'.encode('ascii'))



ax.set_thetamin(0)
ax.set_thetamax(180)
ax.set_ylim(0, 300)

ax_map.set(xlim=[-400, 400], ylim=[-400, 400])
# theta = np.linspace(0, 2 * np.pi, 100)
# values = np.random.rand(100)
theta = [0] * 181
values = [0] * 181
line, = ax.plot(theta, values)

callbacks = CyBotOps()
# axbutton_begin = fig.add_axes([0.7, 0.05, 0.1, 0.075])
# button_begin = Button(axbutton_begin, "Begin")
# button_begin.on_clicked(callbacks.begin)
#
# axbutton_end = fig.add_axes([0.5, 0.05, 0.1, 0.075])
# button_end = Button(axbutton_end, "End")
# button_end.on_clicked(callbacks.end)
#
# axbutton_toggle = fig.add_axes([0.3, 0.05, 0.1, 0.075])
# button_toggle = Button(axbutton_toggle, "Toggle")
# button_toggle.on_clicked(callbacks.toggle)

# axbutton_quit = fig.add_axes([0.1, 0.05, 0.1, 0.075])
# button_quit = Button(axbutton_quit, "Quit")
# button_quit.on_clicked(callbacks.quit)

fig.canvas.mpl_connect('key_press_event', callbacks.key_press)
fig.canvas.mpl_connect('key_release_event', callbacks.key_unpress)


def thread_get_uart():
    while True:
        global moving
        data = soc.recv(9, socket.MSG_WAITALL)
        print(data[4:7].decode('ascii'))
        ax.clear()
        match data[3:4].decode('ascii'):
            case ':':
                angle = int(data[0:3].decode('ascii'))
                distance = int(data[4:7].decode('ascii'))
                angle_rad = angle * (np.pi / 180)
                # theta[angle] = angle * (np.pi / 180)
                theta[angle] = angle_rad
                values[angle] = distance
                # values.append(distance)
                print(angle, ":", distance)
                botTracker.reset()
                if distance < 80. :
                    object_angle = botTracker.cybot_map_angle - (np.pi / 2) + angle_rad
                    object_from_current_x = distance * np.cos(object_angle)
                    object_from_current_y = distance * np.sin(object_angle)
                    ax_map.plot(botTracker.cybot_map_from_org_x + object_from_current_x, botTracker.cybot_map_from_org_y + object_from_current_y, linestyle='None', marker='.', color='blue', markersize=1)

            case 'f':
                distance_forward = int(data[4:7].decode('ascii'))
                botTracker.moved(distance_forward)
            case 'b':
                distance_backward = int(data[4:7].decode('ascii'))
                botTracker.moved(-1 * distance_backward)
            case 'r':
                angle_to_right = int(data[4:7].decode('ascii'))
                botTracker.turned(-1 * angle_to_right)
            case 'l':
                angle_to_left = int(data[4:7].decode('ascii'))
                botTracker.turned(angle_to_left)
            case 'c': # Cliff
                orgx = botTracker.cybot_from_origin_x
                orgy = botTracker.cybot_from_origin_y
                thetaCliff.append(np.arctan2(orgy, orgx))
                valueCliff.append(np.sqrt(orgx * orgx + orgy * orgy))
                ax_map.plot(botTracker.cybot_map_from_org_x, botTracker.cybot_map_from_org_y, linestyle='None', marker='.', color='black', markersize=15)
            case 'e': # Edge
                orgx = botTracker.cybot_from_origin_x
                orgy = botTracker.cybot_from_origin_y
                thetaEdge.append(np.arctan2(orgy, orgx))
                valueEdge.append(np.sqrt(orgx * orgx + orgy * orgy))
                ax_map.plot(botTracker.cybot_map_from_org_x, botTracker.cybot_map_from_org_y, linestyle='None', marker='.', color='yellow', markersize=15)
            case 'o': # Obstacle
                orgx = botTracker.cybot_from_origin_x
                orgy = botTracker.cybot_from_origin_y
                thetaObst.append(np.arctan2(orgy, orgx))
                valueObst.append(np.sqrt(orgx * orgx + orgy * orgy))
                ax_map.plot(botTracker.cybot_map_from_org_x, botTracker.cybot_map_from_org_y, linestyle='None', marker='.', color='red', markersize=15)
                

        ax.plot(theta, values, linestyle='None', marker='.') # markersize=10
        ax.plot(thetaCliff, valueCliff, linestyle='None', marker='.', color='black')
        ax.plot(thetaEdge, valueEdge, linestyle='None', marker='.', color='yellow')
        ax.plot(thetaObst, valueObst, linestyle='None', marker='.', color='red')
        ax.set_thetamin(0)
        ax.set_thetamax(180)
        ax.set_ylim(0, 300)
        orgx = botTracker.cybot_from_origin_x
        orgy = botTracker.cybot_from_origin_y
        print(botTracker.cybot_from_origin_x)
        print(botTracker.cybot_from_origin_y)
        ax.quiver(np.arctan2(orgy, orgx), np.sqrt(orgx * orgx + orgy * orgy), np.cos(botTracker.cybot_angle), np.sin(botTracker.cybot_angle))
        
        plt.draw()
        map()


def map():
    plt.figure(fig_map)

    orgx = botTracker.cybot_map_from_org_x
    orgy = botTracker.cybot_map_from_org_y

    ax_map.quiver(orgx, orgy, np.cos(botTracker.cybot_map_angle), np.sin(botTracker.cybot_map_angle), width=0.005)
    
    # ax_map.plot(botTracker.cybot_map_from_org_x, botTracker.cybot_map_from_org_y, linestyle='None', marker='.')
    plt.draw()
    plt.figure(fig)

try:
    _thread.start_new_thread(thread_get_uart, ())
except:
    print("thread failed")


plt.rcParams["keymap.save"].remove('s')
plt.show()

