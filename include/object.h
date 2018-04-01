struct M_Object_ {
  Vector vtable; // pointer reference to type->info->obj_v_table
  Type type_ref;
  m_uint ref;
  m_bit* data;
};

ANN m_bool initialize_object(const M_Object o, const Type type);
void instantiate_object(const VM_Shred, const Type);
void free_object(const M_Object);
ANEW M_Object new_M_Object(const VM_Shred shred);
//ANEW M_Object new_M_UGen();
ANEW M_Object new_M_Array(Type t, m_uint size, m_uint length, m_uint depth);
ANEW M_Object new_String(const VM_Shred shred, const m_str str);

ANN2(2) void release(M_Object obj, const VM_Shred shred);
ANN void NullException(const VM_Shred shred, const m_str c);

#define STRING(o) *((m_str*)((M_Object)o)->data)
#define ME(o) *((VM_Shred*)((M_Object)o)->data)
#define EV_SHREDS(o) *((Vector*)((M_Object)o)->data)
#define IO_FILE(o)  *(FILE**)(((M_Object)o)->data + SZ_INT)
#define UGEN(o) (*(UGen*)(((M_Object)o)->data))
#define ARRAY(o) (*(M_Vector*)(((M_Object)o)->data))
#define Except(s, c) { NullException(s, c); return; }
