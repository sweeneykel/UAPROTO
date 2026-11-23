# Camera analysis project outline

Current increment: the working Wi-Fi camera baseline has been copied from camera_demo. Open camera_analysis_outline.ino as the Arduino sketch. The camera overlay and board-side RGB sampling are now implemented. The lookup structure and unmatched-result handling are implemented with four user-provided, square-specific prototype ranges. OLED output now selects squares 1-4 using encoder rotation. Encoder sampling and LED integration are deferred.

Design: modular, independent, and as simple as possible. Modules exchange plain
sample/result data and input events. The sketch coordinates them.

## Files and responsibilities

- camera_analysis_outline.ino: current camera/PMU/Wi-Fi setup and reconnect loop.
- app_httpd.cpp and camera_index.h: copied camera web server and browser UI.
- utilities.h: copied board pins and station-mode selection.
- secrets.h: copied local Wi-Fi credentials; excluded from Git.
- project_types.h: shared RGB/sample/result types and event/color definitions.
- camera_data_collection.h/.cpp: camera initialization, browser preview, fixed
  four-square overlay, and average RGB collection from one frame.
- camera_data_processing.h/.cpp: independent RGB range lookup and one string per square.
- camera_lookup_table.h: four user-provided prototype lookup entries.
- camera_sample_api.h/.cpp: browser coordination, held samples, and processing results.
- OLED_display.h/.cpp: initialize OLED and show one selected result/error.
- rotary_encoder_control.h/.cpp: read Twist rotation/press and drive its LED.

## Agreed behavior

Four fixed squares form a 4x1 strip along the longer camera-image dimension,
with small gaps to mimic a pH strip. Present/read them left to right as 1..4.
The grid delineates regions only; there is no color effect. Show RGB readings
on the browser page for prototype evaluation.

An encoder press samples all four regions from one frame. Hold the sample and
results until another press. Rotation selects one of the four results for the
OLED; selection stops at either end and never wraps.

The LED is green once powered on and ready. Every sample press makes it yellow
for 10 seconds. After that period, show red if ANY square has no lookup match;
otherwise green. A new press starts a new sampling/yellow cycle. A mismatch
never turns the LED red before the yellow period ends.

The lookup accepts RGB ranges and returns alphanumeric strings. Each unmatched
square gets its own OLED error indication; its error code is TBD.

## Integration references (unchanged)

- ../camera_demo/: existing camera sketch, web server, and embedded browser UI.
- ../sketch_nov22a/MinimalScreenExample.ino: existing OLED setup and drawing demos.
- ../sketch_nov22a/utilities.h: board pin definitions.

OLED: SSD1306 128x64, U8g2, shared I2C SDA GPIO7 / SCL GPIO6.
Encoder: SparkFun Qwiic Twist with RGB indicator, separate from OLED code.
Coordinate PMU and I2C initialization once when combining the existing examples.
Keep existing OLED boilerplate for now.

Sampling belongs on the board so encoder-triggered analysis and OLED results
can work independently of an open browser. Browser overlay coordinates must
match the regions used by board-side averaging.

## Open questions / decisions for implementation

- Exact RGB ranges, lookup entries, and shared versus per-square tables.
- How overlapping lookup ranges are resolved.
- Error code for a square with no match.
- Exact square size, gaps, positioning, and image orientation.
- Initial OLED content before the first sample.
- Handling capture/hardware failure separately from a lookup mismatch.
- Final Qwiic wiring/address verification and encoder rotation direction.
- Behavior of another press during an active 10-second yellow period.

The camera baseline was copied into this project; source projects remain unchanged.

## Verify this increment

Use the same Arduino board settings that worked for camera_demo. The compilation
check uses ESP32S3 Dev Module, OPI PSRAM, 16MB flash, and default_8MB partitions.
Dependencies: Arduino-ESP32 4.0.0 and XPowersLib.

Upload camera_analysis_outline.ino, open Serial Monitor at 115200 baud, and open
the printed camera URL with the laptop on the same network. Confirm the browser
page, still capture, and streaming work as before. This increment intentionally
preserves the baseline behavior. Compilation alone does not verify hardware.


## Display overlay increment

The camera page now shows four square outlines numbered 1-4 from left to right,
centered in a horizontal row with gaps. At 320x240 they are 64x64 with 8-pixel
gaps. The guides scale with the displayed image, including browser resizing.
They appear over still captures and the stream, without modifying saved images
or camera pixels. RGB sampling is implemented in the next increment described below.

Editable original pages are in web/index_ov*.html. The shared guide is in
web/camera_overlay.html. Run tools/rebuild_camera_pages.ps1 after editing them
to regenerate camera_index.h. All three sensor pages include the same guide.

Upload and hard-refresh the camera page. Start streaming, check the four guides
and gaps, then resize the browser and try a still capture. Confirm placement
before implementing measurement. Existing camera controls remain available.

## RGB sampling increment

Click Take RGB sample to collect four average RGB values from ONE new camera
frame, ordered left to right. Readings appear below the camera image and in
Serial Monitor at 115200 baud. They remain on the ESP32 until the next sample
or reset, and can be restored by refreshing the browser. No persistent logging.

camera_data_collection.cpp performs capture, JPEG conversion, averaging, and
GET/POST /rgb-sample handlers. app_httpd.cpp registers those handlers on the
existing camera-control server. The sketch retains working camera/Wi-Fi setup.
web/camera_overlay.html supplies the button, table, and aligned overlay.

Region geometry uses integer camera pixels: side=min(width/5,height*4/5),
gap=width/40, centered as a four-square row. Integer divisions round down.
This matches the overlay at every image size; gaps are excluded. The overlay
is browser-only and never contaminates the measured camera pixels.
RGB values are rounded channel averages on a 0-255 scale. The JPEG converter's
BGR byte order is explicitly read as B, G, R before reporting R, G, B.

Use QVGA (320x240) first. Higher resolutions need more PSRAM and take longer.
If capture/allocation/conversion fails, previous readings remain unchanged.
The stream may briefly pause during capture. Automatic exposure/white balance
and JPEG compression affect readings; they are not calibrated color values.

Hardware verification: place red, green, blue, and white patches under squares
1-4. Sample, confirm channel order, move the patches, and confirm the values
stay held until sampling again. Check that changing only the gaps does not
affect the region averages. Refresh the page to verify held results return.
Check a still capture and stream after sampling. Lookup/OLED/encoder/LED logic
has not been integrated in this increment.
## Lookup structure and unmatched results

camera_lookup_table.h is the only calibration table to edit. It is intentionally
EMPTY: every captured square currently returns NoMatch, with the descriptive
placeholder "NO MATCH - code TBD". This is not a finalized error code.
An empty table does not pretend that prototype colors have calibrated meanings.

Each entry specifies square 1..4 (0 = all squares), inclusive minimum RGB,
inclusive maximum RGB, and a result string. All three channels must be within
range. The file contains a commented structural example; it is not active.
Change the std::array entry count when adding real measured entries.
First matching entry wins (provisional overlap rule). An inverted range cannot
match; null/empty output strings and entries for other squares are ignored.

camera_data_processing.h/.cpp perform lookup with no camera, HTTP, OLED, or
encoder calls. hasMismatch returns true if any square is NoMatch. NotSampled
remains distinct from NoMatch. The explicit-table processSample overload lets
a test use known ranges without inserting fake calibration data into firmware.

camera_sample_api.h/.cpp coordinate collection and processing for the existing
/rgb-sample GET/POST endpoint. RGB readings, strings, per-square matched/no_match
statuses, and an overall mismatch boolean are returned together and held until
the next successful sample. Capture failure retains previous RGB AND results.
The browser displays strings beside RGB; Serial Monitor prints each result.
No sample yet still returns 404, not four invented mismatch results.
Sample dimensions are now included in the shared Sample type.

After upload, hard-refresh and sample: expect four "NO MATCH - code TBD" results
with the empty table. Confirm RGB and results stay held, survive browser refresh,
and camera streaming still works. No LED, OLED, or encoder behavior is added.

## First populated lookup entries

The table now contains the four ranges provided by the user, without adjustment:

| Square | Minimum RGB | Maximum RGB | Result |
| --- | --- | --- | --- |
| 1 | 157, 172, 188 | 165, 180, 196 | Leukocytes - Negative |
| 2 | 202, 216, 226 | 210, 224, 234 | pH - 7.0 |
| 3 | 196, 192, 188 | 204, 200, 196 | Protein - Negative |
| 4 | 163, 160, 144 | 171, 168, 152 | Nitrite - Negative |

These replace the earlier empty-table state. All three channels must be inside
that square's inclusive range to match. A reading outside its range continues
to show "NO MATCH - code TBD". Upload, hard-refresh, and take a new sample to
test the populated table; held readings/results do not update before sampling.
## OLED increment: fixed square 1

OLED_display.h/.cpp now use the existing example's U8g2 SSD1306 128x64 full
buffer and U8G2_R2 orientation. The sketch initializes the display after the
shared PMU/Wire setup on SDA GPIO7 / SCL GPIO6, without a second PMU instance.
The screen is probed at 0x3C; absence is logged but does not stop the camera.
U8g2 is an additional dependency, already installed for the original OLED demo.

On startup the screen shows "Square 1 / 4", "Not sampled", and instructions.
A successful browser sample updates it with square 1's match status and full
lookup string or "NO MATCH - code TBD". It holds that content until the next
successful sample. Failed captures leave it unchanged. This fixed selection is
temporary; rotary encoder input, selection changes, and LED control are not
implemented yet.

Text uses 6x10 font with word wrapping in three lines; longer-than-screen strings
end with an ellipsis. Current configured result strings fit. Screen updates
are performed only at initialization and after a sample, not every loop.
Original sketch_nov22a OLED boilerplate remains untouched.

Hardware verification: upload, check startup text/orientation, then take a
browser sample. Confirm square 1's string matches the browser result. Change
square 1 to an unmatched color and sample again to check the placeholder error.
Confirm the screen holds until another successful capture, and verify browser
streaming still works. Other squares remain visible in the browser only.
## Encoder increment 1: input verification only

rotary_encoder_control owns its private TWIST instance and uses the existing
I2C bus (SDA7/SCL6, Twist default address 0x3F). begin returns false if absent
instead of freezing. Wiring changes require a restart to initialize the encoder.

pollEvent returns one Next/Previous per accumulated hardware detent, or one
SamplePressed for a completed click. Clicks take priority; rotation remains
queued. Positive hardware delta maps to Next; negative maps to Previous.
Input is normally polled about every 10ms. Existing Wi-Fi reconnection may still
pause the main loop during a network outage; the hardware delta/click flag latch
input, although multiple clicks during a long pause can coalesce into one.

The sketch only logs events to Serial Monitor at 115200:
- Encoder: Next
- Encoder: Previous
- Encoder: SamplePressed (test only)

Test one detent each way, several detents quickly, a short click, and a long
hold/release (one click event). Confirm camera streaming, browser sampling,
lookup, and square 1 OLED display still work. With the encoder unplugged before
boot, the camera/OLED should continue and Serial should report encoder absence.

OLED selection, encoder-triggered sampling, and indicator LED behavior are
intentionally deferred to separate increments.

## Encoder wiring correction: GPIO15 / GPIO16

The encoder is physically wired to SDA GPIO15 and SCL GPIO16. GPIO7/6 are not
exposed for rewiring. Both ESP32-S3 hardware I2C controllers are already occupied:
OLED/PMU on GPIO7/6 and camera SCCB on GPIO5/4. Do not start Wire1 on GPIO15/16.

encoder_software_i2c.h/.cpp implement a dedicated open-drain software bus on the
actual encoder wiring, with ACK checks, clock-stretch timeout, and startup bus
recovery. No hardware controller is allocated or remapped. The encoder needs
3.3V, common GND, and pull-ups (normally present on the Qwiic board).

rotary_encoder_control now reads the Twist register protocol directly: address
0x3F, status 0x01, little-endian absolute count 0x05, clicked flag bit 2. It uses
signed differences with 16-bit wrap handling instead of reading and clearing the
hardware delta register. Failed reads do not generate fake input events.
The SparkFun Arduino library is no longer needed by the sketch; its installed
source was used to verify the register protocol.

This supersedes the prior shared-Wire wiring description. Upload and check for
"Encoder software I2C: SDA GPIO15, SCL GPIO16, address 0x3F" followed by
"Encoder ready." Verify Previous/Next/click events and confirm the OLED/camera
still work. If detection still fails, check power/GND, SDA/SCL order, pull-ups,
and whether the device's address was changed from its 0x3F default.

Scope remains input testing only; no selection, capture, or LED actions.

## Encoder increment 2: OLED result selection

Rotate to select the held result for square 1, 2, 3, or 4. Previous at square 1
stays at 1; Next at square 4 stays at 4. Reversing direction immediately moves
away from an endpoint. Selection is initially square 1, resets on restart, and
is retained when a new browser sample arrives. The OLED title identifies the
selected square, followed by its matched string or unmatched placeholder.

Before the first sample, rotation changes the square title while retaining the
Not sampled message. A failed capture leaves both held results and selection
unchanged. The browser continues to show all four results as before.

camera_sample_api::selectRelative coordinates selection and display. A FreeRTOS
mutex protects selection/result copies and OLED rendering across the Arduino
loop and browser server task, preventing concurrent U8g2 buffer updates.
Camera collection, lookup processing, OLED rendering, and encoder input retain
their existing responsibilities.

Hardware verification:
1. Upload and take a browser RGB sample.
2. Rotate across all four squares; compare each OLED string with its browser row.
3. Keep turning at both ends: confirm no wrapping, then reverse one detent.
4. With square 3 selected, take a new browser sample: it must remain square 3.
5. Verify unmatched results, streaming, and rotation before a first sample.
Encoder clicks remain Serial-only test events; LED behavior is unchanged.