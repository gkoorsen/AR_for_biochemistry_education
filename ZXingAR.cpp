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
        var cameraTexture = new Texture2D(arCamera.pixelWidth, arCamera.pixelHeight, TextureFormat.RGB24, false);
        cameraTexture.ReadPixels(new Rect(0, 0, arCamera.pixelWidth, arCamera.pixelHeight), 0, 0);
        cameraTexture.Apply();

        var luminanceSource = new RGBLuminanceSource(cameraTexture.GetRawTextureData(), cameraTexture.width, cameraTexture.height);
        var binaryBitmap = new BinaryBitmap(new HybridBinarizer(luminanceSource));

        var result = barcodeReader.Decode(binaryBitmap);
        if (result != null)
        {
            ShowModel(result.Text);
        }

        if (Input.touchCount > 0)
        {
            Touch touch = Input.GetTouch(0);

            if (touch.phase == TouchPhase.Began)
            {
                startTouchPosition = touch.position;
                if (touch.tapCount == 1)
                {
                    rotationSpeed += 0.5f;
                }
                else if (touch.tapCount == 2)
                {
                    rotationSpeed -= 0.5f;

                    if (rotationSpeed < 0)
                    {
                        rotationSpeed = 0;
                    }
                }
            }
            else if (touch.phase == TouchPhase.Ended)
            {
                endTouchPosition = touch.position;

                if (Mathf.Abs(endTouchPosition.y - startTouchPosition.y) > 50 && Mathf.Abs(endTouchPosition.x - startTouchPosition.x) < 50)
                {
                    if (endTouchPosition.y > startTouchPosition.y)
                    {
                        OnSwipeUp();
                    }
                    else
                    {
                        OnSwipeDown();
                    }
                }
            }
        }

        if (currentModelName != "")
        {
            ARObjects[currentModelName].transform.Rotate(0, rotationSpeed, 0);
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
