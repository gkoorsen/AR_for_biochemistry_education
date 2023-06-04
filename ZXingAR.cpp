using UnityEngine;
using UnityEngine.XR.ARFoundation;
using UnityEngine.XR.ARSubsystems;
using ZXing;
using ZXing.Common;
using ZXing.QrCode;

public class QRCodeDetection : MonoBehaviour
{
    private Camera arCamera;
    private BarcodeReader barcodeReader;
}

void Start()
{
    arCamera = GetComponent<ARSessionOrigin>().camera;

    // Initialize the ZXing barcode reader
    barcodeReader = new BarcodeReader();
    barcodeReader.Options = new DecodingOptions
    {
        TryHarder = true,
        PossibleFormats = new[] { BarcodeFormat.QR_CODE }
    };
}

void Update()
{
    // Capture a camera frame
    var cameraTexture = new Texture2D(arCamera.pixelWidth, arCamera.pixelHeight, TextureFormat.RGB24, false);
    cameraTexture.ReadPixels(new Rect(0, 0, arCamera.pixelWidth, arCamera.pixelHeight), 0, 0);
    cameraTexture.Apply();

    // Convert the camera frame to a ZXing luminance source
    var luminanceSource = new RGBLuminanceSource(cameraTexture.GetRawTextureData(), cameraTexture.width, cameraTexture.height);
    var binaryBitmap = new BinaryBitmap(new HybridBinarizer(luminanceSource));

    // Decode QR codes from the binary bitmap
    var result = barcodeReader.Decode(binaryBitmap);
    if (result != null)
    {
        // QR code detected and decoded, perform your desired actions here
        Debug.Log("QR Code detected: " + result.Text);
    }
}
