using UnityEngine;
using UnityEngine.XR.ARFoundation;
using UnityEngine.XR.ARSubsystems;
using ZXing;
using ZXing.Common;
using ZXing.QrCode;
using System.Collections.Generic;

public class QRCodeDetection : MonoBehaviour
{
    private Camera arCamera;
    private BarcodeReader barcodeReader;
    private string currentModelName = "";
    private float rotationSpeed = 0.5f;
    private Vector2 startTouchPosition, endTouchPosition;

    public List<GameObject> modelPrefabs;
    public Dictionary<string, GameObject> ARObjects = new Dictionary<string, GameObject>();

    void Awake()
    {
        // Load all models from the Resources/Models directory
        modelPrefabs = new List<GameObject>(Resources.LoadAll<GameObject>("Models"));

        // Sort the models by name
        modelPrefabs.Sort((a, b) => a.name.CompareTo(b.name));

        foreach (GameObject prefab in modelPrefabs)
        {
            GameObject newObject = Instantiate(prefab, Vector3.zero, Quaternion.identity);
            newObject.name = prefab.name;
            newObject.SetActive(false);
            ARObjects.Add(prefab.name, newObject);
        }
    }

    void Start()
    {
        arCamera = GetComponent<ARSessionOrigin>().camera;

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
        var cameraTexture = new Texture2D(arCamera.pixelWidth, arCamera.pixelHeight);
        arCamera.Render();
        RenderTexture.active = arCamera.targetTexture;
        cameraTexture.ReadPixels(new Rect(0, 0, arCamera.pixelWidth, arCamera.pixelHeight), 0, 0);
        cameraTexture.Apply();

        // Convert the camera frame to a ZXing luminance source
        var luminanceSource = new RGBLuminanceSource(cameraTexture.GetRawTextureData(), cameraTexture.width, cameraTexture.height);

        // Decode QR codes from the luminance source
        var result = barcodeReader.Decode(luminanceSource);
        if (result != null)
        {
            // QR code detected and decoded, perform your desired actions here
            Debug.Log("QR Code detected: " + result.Text);

            if (!qrDetected)
            {
                qrDetected = true;
                ShowModel(modelPrefabs[0].name);
            }
        }

        // Clean up
        Destroy(cameraTexture);

        // Touch inputs for switching between models and changing rotation speed
        if (Input.touchCount > 0)
        {
            Touch touch = Input.GetTouch(0);

            // Check for swipe up
            if (touch.phase == TouchPhase.Ended && touch.deltaPosition.y > 0)
            {
                SwitchToNextModel();
            }

            // Check for swipe down
            if (touch.phase == TouchPhase.Ended && touch.deltaPosition.y < 0)
            {
                SwitchToPreviousModel();
            }

            // Single tap to speed up rotation
            if (touch.tapCount == 1)
            {
                RotateSpeed += 10f;
            }

            // Double tap to slow down rotation
            if (touch.tapCount == 2)
            {
                RotateSpeed -= 10f;
                if (RotateSpeed < 0)
                {
                    RotateSpeed = 0;
                }
            }
        }
    }
    
    void ShowModel(string modelName)
    {
        if (currentModelName != "")
        {
            ARObjects[currentModelName].SetActive(false);
        }

        if (ARObjects.ContainsKey(modelName))
        {
            ARObjects[modelName].SetActive(true);
            currentModelName = modelName;
        }
    }

    public void OnSwipeUp()
    {
        int currentIndex = modelPrefabs.FindIndex(prefab => prefab.name == currentModelName);
        int nextIndex = (currentIndex + 1) % modelPrefabs.Count;
        ShowModel(modelPrefabs[nextIndex].name);
    }

    public void OnSwipeDown()
    {
        int currentIndex = modelPrefabs.FindIndex(prefab => prefab.name == currentModelName);
        int previousIndex = (currentIndex - 1 + modelPrefabs.Count) % modelPrefabs.Count;
        ShowModel(modelPrefabs[previousIndex].name);
    }
}
