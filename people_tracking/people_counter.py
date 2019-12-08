# import the necessary packages
from pyimagesearch.centroidtracker import CentroidTracker
from pyimagesearch.trackableobject import TrackableObject
from keras import backend as K
from keras.preprocessing import image
from keras.optimizers import Adam

from SSD.models.keras_ssd300 import ssd_300
from SSD.keras_loss_function.keras_ssd_loss import SSDLoss

import numpy as np
import dlib
import cv2
import os

# Neural Netowrks settings and load

# Set the image size.
img_height = 300
img_width = 300

# 1: Build the Keras model

K.clear_session()  # Clear previous models from memory.

model = ssd_300(image_size=(img_height, img_width, 3),
                n_classes=20,
                mode='inference',
                l2_regularization=0.0005,
                scales=[0.1, 0.2, 0.37, 0.54, 0.71, 0.88, 1.05],
                # The scales for MS COCO are [0.07, 0.15, 0.33, 0.51, 0.69, 0.87, 1.05]
                aspect_ratios_per_layer=[[1.0, 2.0, 0.5],
                                         [1.0, 2.0, 0.5, 3.0, 1.0 / 3.0],
                                         [1.0, 2.0, 0.5, 3.0, 1.0 / 3.0],
                                         [1.0, 2.0, 0.5, 3.0, 1.0 / 3.0],
                                         [1.0, 2.0, 0.5],
                                         [1.0, 2.0, 0.5]],
                two_boxes_for_ar1=True,
                steps=[8, 16, 32, 64, 100, 300],
                offsets=[0.5, 0.5, 0.5, 0.5, 0.5, 0.5],
                clip_boxes=False,
                variances=[0.1, 0.1, 0.2, 0.2],
                normalize_coords=True,
                subtract_mean=[123, 117, 104],
                swap_channels=[2, 1, 0],
                confidence_thresh=0.5,
                iou_threshold=0.45,
                top_k=200,
                nms_max_output_size=400)

# 2: Load the trained weights into the model.

weights_path = 'SSD/VGG_VOC0712_SSD_300x300_ft_iter_120000.h5'

model.load_weights(weights_path, by_name=True)

# 3: Compile the model so that Keras won't complain the next time you load it.

adam = Adam(lr=0.001, beta_1=0.9, beta_2=0.999, epsilon=1e-08, decay=0.0)

ssd_loss = SSDLoss(neg_pos_ratio=3, alpha=1.0)

model.compile(optimizer=adam, loss=ssd_loss.compute_loss)

# initialize the video writer (we'll instantiate later if need be)
writer = None

# initialize the frame dimensions (we'll set them as soon as we read
# the first frame from the video)
W = img_width
H = img_height

# instantiate our centroid tracker, then initialize a list to store
# each of our dlib correlation trackers, followed by a dictionary to
# map each unique object ID to a TrackableObject
ct = CentroidTracker(maxDisappeared=1, maxDistance=100)
trackers = []
trackableObjects = {}

# initialize the total number of frames processed thus far, along
# with the total number of objects that have moved either up or down
totalFrames = 0
totalDown = 0
totalUp = 0
input_images = []
rgb = None
frame = None
clean_frame = None
img = []
# loop over frames from the video stream
while True:
    fromdir = '../server/img/'
    gt = os.path.getmtime  # change if you want something else
    img_path = ''

    elems = [(fromdir + f, gt(fromdir + f)) for f in os.listdir(fromdir) if f != '.DS_Store']
    if len(elems) != 0:
        img_path = min(elems)[0]
        # We'll only load one image in this example.
        input_images = []
        print(img_path)
        frame = cv2.imread(img_path)
        frame = cv2.resize(frame, (img_height, img_width))
        clean_frame = frame.copy()
        rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        img = image.img_to_array(image.load_img(img_path, target_size=(img_height, img_width)))
        os.remove(img_path)
    else:
        frame = clean_frame.copy()
    # img = image.img_to_array(frame)
    # input_images.append(img)
    # input_images = np.array(input_images)

    # if we are supposed to be writing a video to disk, initialize
    # the writer

    # initialize the current status along with our list of bounding
    # box rectangles returned by either (1) our object detector or
    # (2) the correlation trackers
    status = "Waiting"
    rects = []

    # check to see if we should run a more computationally expensive
    # object detection method to aid our tracker
    if img_path != '':
        # set the status and initialize our new set of object trackers
        status = "Detecting"
        trackers = []

        input_images.append(img)
        input_images = np.array(input_images)

        y_pred = model.predict(input_images)

        confidence_threshold = 0.5

        y_pred_thresh = [y_pred[k][y_pred[k, :, 1] > confidence_threshold] for k in range(y_pred.shape[0])]

        np.set_printoptions(precision=2, suppress=True, linewidth=90)

        persons = []
        for box in y_pred_thresh[0]:
            if box[0] == 15:
                persons.append(box)

        for box in persons:
            xmin = box[2]
            ymin = box[3]
            xmax = box[4]
            ymax = box[5]
            # construct a dlib rectangle object from the bounding
            # box coordinates and then start the dlib correlation
            # tracker
            tracker = dlib.correlation_tracker()

            rect = dlib.rectangle(int(xmin), int(ymin), int(xmax), int(ymax))
            # left, top, right, bottom
            # rects.append((int(xmin), int(ymin), int(xmax), int(ymax)))

            tracker.start_track(rgb, rect)

            # add the tracker to our list of trackers so we can
            # utilize it during skip frames
            trackers.append(tracker)


    # otherwise, we should utilize our object *trackers* rather than
    # object *detectors* to obtain a higher frame processing throughput
    else:
        # loop over the trackers
        for tracker in trackers:
            # set the status of our system to be 'tracking' rather
            # than 'waiting' or 'detecting'
            status = "Tracking"

            # update the tracker and grab the updated position
            tracker.update(rgb)
            pos = tracker.get_position()

            # unpack the position object
            startX = int(pos.left())
            startY = int(pos.top())
            endX = int(pos.right())
            endY = int(pos.bottom())

            # add the bounding box coordinates to the rectangles list
            rects.append((startX, startY, endX, endY))

    # draw a horizontal line in the center of the frame -- once an
    # object crosses this line we will determine whether they were
    # moving 'up' or 'down'
    cv2.line(frame, (0, H // 2), (W, H // 2), (0, 255, 255), 2)

    # use the centroid tracker to associate the (1) old object
    # centroids with (2) the newly computed object centroids
    objects = ct.update(rects)

    # loop over the tracked objects
    for (objectID, centroid) in objects.items():
        # check to see if a trackable object exists for the current
        # object ID
        to = trackableObjects.get(objectID, None)

        # if there is no existing trackable object, create one
        if to is None:
            to = TrackableObject(objectID, centroid)

        # otherwise, there is a trackable object so we can utilize it
        # to determine direction
        else:
            # the difference between the y-coordinate of the *current*
            # centroid and the mean of *previous* centroids will tell
            # us in which direction the object is moving (negative for
            # 'up' and positive for 'down')
            y = [c[1] for c in to.centroids]
            direction = centroid[1] - np.mean(y)
            to.centroids.append(centroid)

            # check to see if the object has been counted or not
            if not to.counted:
                # if the direction is negative (indicating the object
                # is moving up) AND the centroid is above the center
                # line, count the object
                if direction < 0 and centroid[1] < H // 2:
                    totalUp += 1
                    to.counted = True

                # if the direction is positive (indicating the object
                # is moving down) AND the centroid is below the
                # center line, count the object
                elif direction > 0 and centroid[1] > H // 2:
                    totalDown += 1
                    to.counted = True

        # store the trackable object in our dictionary
        trackableObjects[objectID] = to

        # draw both the ID of the object and the centroid of the
        # object on the output frame
        text = "ID {}".format(objectID)
        cv2.putText(frame, text, (centroid[0] - 10, centroid[1] - 10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
        cv2.circle(frame, (centroid[0], centroid[1]), 4, (0, 255, 0), -1)

    # construct a tuple of information we will be displaying on the
    # frame
    info = [
        ("Up", totalUp),
        ("Down", totalDown),
        ("Status", status),
    ]

    # loop over the info tuples and draw them on our frame
    for (i, (k, v)) in enumerate(info):
        text = "{}: {}".format(k, v)
        cv2.putText(frame, text, (10, H - ((i * 20) + 20)),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.4, (0, 255, 0), 1)

    # show the output frame
    cv2.imshow("Frame", frame)
    key = cv2.waitKey(1) & 0xFF

    # if the `q` key was pressed, break from the loop
    if key == ord("q"):
        break

    # increment the total number of frames processed thus far and
    # then update the FPS counter
    totalFrames += 1

# close any open windows
cv2.destroyAllWindows()
