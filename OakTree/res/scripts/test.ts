export class Test {
    /**
     * @param {strint} a
     */
    public a: string;

    private b: number;
    public c: number;
    public d: object;

    constructor(a: string) {
        this.a = a;
    }

    public static test() {
        return "This is a test!";
    }
}