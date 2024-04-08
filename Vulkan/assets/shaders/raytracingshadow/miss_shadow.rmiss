struct PayloadShadow
{
    bool shadowed;
};

[shader("miss")]
void main(inout PayloadShadow io_PayloadShadow)
{
    io_PayloadShadow.shadowed = false;
}