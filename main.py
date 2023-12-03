import cv2
import numpy as np

# Video capture setup
videoCapture = cv2.VideoCapture(0)
videoCapture.set(3, 160)
videoCapture.set(4, 120)

# Video mask setup
low_b = np.uint8([100,100,100])
high_b = np.uint8([0,0,0])

# Colors
white = (255,255,255)
red = (0,0,255)

def turnLeft():
    print("Turn left")
    # TODO code to make car turn left

def onTrack():
    print("On Track")
    # TODO code to make car keep going foward

def turnRight():
    print("Turn Right")
    # TODO code to make car turn right

def main():
    ret, frame = videoCapture.read()
    width  = int(videoCapture.get(3))  # float `width`
    height = int(videoCapture.get(4))  # float `height`
    center = int(width/2)
    centerRange = int(width*0.15)
    LeftRange = center - centerRange
    RightRange = center + centerRange

    mask = cv2.inRange(frame, high_b, low_b)
    countors, hierarchy = cv2.findContours(mask, 1, cv2.CHAIN_APPROX_NONE)
    if len(countors) > 0:
        c = max(countors, key=cv2.contourArea)
        M = cv2.moments(c)
        if M['m00'] != 0:
            cx = int(M['m10']/M['m00'])
            cy = int(M['m01']/M['m00'])
            if cx >= RightRange:
                turnRight()
            if cx < RightRange and cx > LeftRange:
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
