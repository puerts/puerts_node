declare module 'puerts_node' {
    interface $Ref<T> {
        __doNoAccess: T
    }

    interface $InRef<T> {
        __doNoAccess: T
    }
    
    type $Nullable<T> = T | null;

    type cstring = string | ArrayBuffer;
    
    export function $ref<T>(x? : T) : $Ref<T>;
    
    export function $unref<T>(x: $Ref<T> | $InRef<T>) : T;
    
    export function $set<T>(x: $Ref<T> | $InRef<T>, val:T) : void;
    
	export function load<T>(dllpath): T;
    
    export function declaration(): string;
}