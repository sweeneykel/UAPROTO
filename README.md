# Camera analysis project outline

This is a structural outline, not working firmware. No hardware libraries beyond
Arduino are integrated; all module functions are placeholders. Open
camera_analysis_outline.ino as the Arduino sketch.

Design: modular, independent, and as simple as possible. Modules exchange plain
sample/result data and input events. The sketch coordinates them.

## Files and responsibilities

- camera_analysis_outline.ino: future setup, coordination, selected square,
  held results, and nonblocking LED timing.
- project_types.h: shared RGB/sample/result types and event/color definitions.
- camera_data_collection.h/.cpp: camera initialization, browser preview, fixed
  four-square overlay, and average RGB collection from one frame.
- camera_data_processing.h/.cpp: RGB range lookup and one string per square.
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

No existing camera or OLED files have been changed or copied into this outline.
