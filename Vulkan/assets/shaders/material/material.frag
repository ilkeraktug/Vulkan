struct MaterialBuffer
{
    float ID;
};

[[vk::push_constant]] MaterialBuffer material;

struct FSOutput
{
    float first : SV_TARGET0;
};

FSOutput main()
{
    FSOutput output = (FSOutput)0;
    
    output.first = material.ID;
    //output.first = 1.71f;
    
    return output;
}