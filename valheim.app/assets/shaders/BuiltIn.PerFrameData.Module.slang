module per_frame_data;

public struct PerFrameData {
  public float4x4 projection;
  public float4x4 view;
  public float4x4 model;
}

[vk_push_constant]
public PerFrameData frameData;