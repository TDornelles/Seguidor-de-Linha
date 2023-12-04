import cv2
import numpy as np
import requests
import grequests

async_list = []
lastRequest = ""


# http://192.168.15.43/on?R=95
# http://192.168.15.43/on?L=95

# para acionar o servo: colocar um valor entre 0 e 180 tanto para o da esquerda quanto o da direita.
# valores abaixo de 95 roda em um sentido e acima no oposto.


def ligaR(potencia):
    resp = ('http://192.168.15.74/on?R=' + potencia)
    return resp


def ligaL(potencia):
    resp = ('http://192.168.15.74/on?L=' + potencia)
    return resp


def pararR():
    resp = 'http://192.168.15.74/on?R=95'
    return resp


def pararL():
    resp = 'http://192.168.15.74/on?L=95'
    return resp


def mandarRequest(req):
    action_item = grequests.get(req)
    # Add the task to our list of things to do via async
    async_list.append(action_item)
    grequests.map(async_list, gtimeout=0.2)
    async_list.clear()


# Video capture setup
videoCapture = cv2.VideoCapture(0)
videoCapture.set(3, 160)
videoCapture.set(4, 120)

# Video mask setup
low_b = np.uint8([100, 100, 100])
high_b = np.uint8([0, 0, 0])

# Colors
white = (255, 255, 255)
red = (0, 0, 255)


def turnLeft():
    global lastRequest
    if lastRequest != 'left':
        print("Turn left")
        mandarRequest(pararL())
        mandarRequest(ligaR('110'))
        lastRequest = 'left'


def onTrack():
    global lastRequest
    if lastRequest != 'forward':
        print("On Track")
        mandarRequest(ligaR('110'))
        mandarRequest(ligaL('110'))
        lastRequest = 'forward'


def turnRight():
    global lastRequest
    if lastRequest != 'right':
        print("Turn Right")
        mandarRequest(pararR())
        mandarRequest(ligaL('110'))
        lastRequest = 'right'


def main():
    ret, frame = videoCapture.read()
    width = int(videoCapture.get(3))  # float `width`
    height = int(videoCapture.get(4))  # float `height`
    center = int(width / 2)
    centerRange = int(width * 0.15)
    LeftRange = center - centerRange
    RightRange = center + centerRange

    mask = cv2.inRange(frame, high_b, low_b)
    countors, hierarchy = cv2.findContours(mask, 1, cv2.CHAIN_APPROX_NONE)
    if len(countors) > 0:
        c = max(countors, key=cv2.contourArea)
        M = cv2.moments(c)
        if M['m00'] != 0:
            cx = int(M['m10'] / M['m00'])
            cy = int(M['m01'] / M['m00'])
            print('cx: {cx}')
            if cx >= RightRange:
                turnRight()

            if RightRange > cx > LeftRange:
                onTrack()

            if cx <= LeftRange:
                turnLeft()

            cv2.circle(frame, (cx, cy), 5, white, -1)
            cv2.line(frame, (RightRange, 0), (RightRange, height), red, 2)
            cv2.line(frame, (LeftRange, 0), (LeftRange, height), red, 2)

    # cv2.drawContours(frame, countors, -1, (0,255,0), 1)
    # cv2.imshow("Mask", mask)
    cv2.imshow("Frame", frame)


while True:
    main()
    if cv2.waitKey(1) & 0xff == ord('q'):
        videoCapture.release()
        cv2.destroyAllWindows()
        break
